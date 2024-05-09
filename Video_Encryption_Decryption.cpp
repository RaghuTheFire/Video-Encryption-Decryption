// Include necessary libraries
#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_File_Chooser.H>
#include <FL/Fl_Text_Editor.H>
#include <opencv2/opencv.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>

using namespace cv;
using namespace std;

// Global variables
string filename;
vector<string> path_list;

// Function prototypes
void encrypt_fun(Fl_Text_Editor* path_text);
void decrypt_fun(string filename);
void reset_fun(string filename);

// Main window
Fl_Window* window = new Fl_Window(1000, 700, "Video Encryption Decryption");

// Top label
Fl_Box* start1 = new Fl_Box(120, 10, 760, 100, "VIDEO  ENCRYPTION\nDECRYPTION");
start1->labelfont(FL_TIMES_BOLD_ITALIC);
start1->labelsize(55);
start1->labelcolor(FL_MAGENTA);

// Selected video label
Fl_Box* lbl2 = new Fl_Box(80, 220, 200, 30, "Selected Video");
lbl2->labelfont(FL_TIMES_BOLD);
lbl2->labelsize(30);
lbl2->labelcolor(FL_BROWN);

// Text editor for displaying selected video path
Fl_Text_Editor* path_text = new Fl_Text_Editor(80, 270, 740, 90);
path_text->textfont(FL_TIMES_BOLD);
path_text->textsize(30);
path_text->textcolor(FL_ORANGE);
path_text->color(FL_YELLOW);

// Encrypt video button
Fl_Button* encrypt_button = new Fl_Button(120, 450, 250, 50, "ENCRYPT VIDEO");
encrypt_button->labelfont(FL_TIMES_BOLD);
encrypt_button->labelsize(25);
encrypt_button->color(FL_ORANGE);
encrypt_button->labelcolor(FL_BLUE);
encrypt_button->callback((Fl_Callback*)encrypt_fun, path_text);

// Decrypt video button
Fl_Button* decrypt_button = new Fl_Button(550, 450, 250, 50, "DECRYPT VIDEO");
decrypt_button->labelfont(FL_TIMES_BOLD);
decrypt_button->labelsize(25);
decrypt_button->color(FL_ORANGE);
decrypt_button->labelcolor(FL_BLUE);
decrypt_button->callback((Fl_Callback*)decrypt_fun, filename);

// Select video button
Fl_Button* select_button = new Fl_Button(80, 580, 150, 50, "SELECT");
select_button->labelfont(FL_TIMES_BOLD);
select_button->labelsize(25);
select_button->color(FL_GREEN);
select_button->labelcolor(FL_BLUE);
select_button->callback((Fl_Callback*)fl_file_chooser, "*.mp4", "Select Video File", NULL, NULL);

// Reset video button
Fl_Button* reset_button = new Fl_Button(420, 580, 150, 50, "RESET");
reset_button->labelfont(FL_TIMES_BOLD);
reset_button->labelsize(25);
reset_button->color(FL_YELLOW);
reset_button->labelcolor(FL_BLUE);
reset_button->callback((Fl_Callback*)reset_fun, filename);

// Exit button
Fl_Button* exit_button = new Fl_Button(780, 580, 150, 50, "EXIT");
exit_button->labelfont(FL_TIMES_BOLD);
exit_button->labelsize(25);
exit_button->color(FL_RED);
exit_button->labelcolor(FL_BLUE);
exit_button->callback((Fl_Callback*)exit, 0);

window->end();
window->show();

// Function to encrypt video
void encrypt_fun(Fl_Text_Editor* path_text) {
    filename = path_text->value();

    // Convert video to images
    VideoCapture cap(filename);
    double fps = cap.get(CAP_PROP_FPS);
    int frame_count = 0;

    if (!cap.isOpened()) {
        cout << "Error opening video stream or file" << endl;
        return;
    }

    if (!fs::exists("Video Images")) {
        fs::create_directory("Video Images");
    }

    while (true) {
        Mat frame;
        bool success = cap.read(frame);

        if (!success) {
            cout << "Error capturing frame" << endl;
            break;
        }

        if (frame_count % static_cast<int>(fps) == 0) {
            int frame_number = frame_count / static_cast<int>(fps);
            string frame_name = "Video Images/frame" + to_string(frame_number) + ".jpg";
            imwrite(frame_name, frame);

            // Encrypt the image
            Mat image_input = imread(frame_name, IMREAD_GRAYSCALE);
            image_input.convertTo(image_input, CV_64FC1, 1.0 / 255.0);
            int rows = image_input.rows;
            int cols = image_input.cols;

            double mu = 0, sigma = 0.1;
            Mat key = Mat::zeros(rows, cols, CV_64FC1);
            randn(key, mu, sigma);
            key += 1e-6; // Add a small constant to avoid division by zero

            Mat image_encrypted;
            divide(image_input, key, image_encrypted);
            image_encrypted.convertTo(image_encrypted, CV_8UC1, 255.0);
            imwrite(frame_name, image_encrypted);
        }

        frame_count++;
    }

    cap.release();

    // Create encrypted video from images
    vector<Mat> frames;
    for (int i = 0; i < frame_count / static_cast<int>(fps); i++) {
        string frame_name = "Video Images/frame" + to_string(i) + ".jpg";
        Mat frame = imread(frame_name);
        frames.push_back(frame);
    }

    VideoWriter writer("encrypted_video.mp4", VideoWriter::fourcc('m', 'p', '4', 'v'), fps, frames[0].size());
    for (const auto& frame : frames) {
        writer.write(frame);
    }
    writer.release();

    // Play encrypted video
    VideoCapture encrypted_cap("encrypted_video.mp4");
    if (!encrypted_cap.isOpened()) {
        cout << "Error opening encrypted video" << endl;
        return;
    }

    namedWindow("Encrypted Video", WINDOW_AUTOSIZE);
    while (true) {
        Mat frame;
        bool success = encrypted_cap.read(frame);

        if (!success) {
            cout << "Error capturing frame from encrypted video" << endl;
            break;
        }

        imshow("Encrypted Video", frame);
        if (waitKey(30) >= 0) {
            break;
        }
    }

    encrypted_cap.release();
    destroyAllWindows();
}

// Function to decrypt video
void decrypt_fun(string filename) {
    VideoCapture cap(filename);

    if (!cap.isOpened()) {
        cout << "Error opening video stream or file" << endl;
        return;
    }

    namedWindow("Decrypted Video", WINDOW_AUTOSIZE);
    while (true) {
        Mat frame;
        bool success = cap.read(frame);

        if (!success) {
            cout << "Error capturing frame from video" << endl;
            break;
        }

        Mat gray_frame;
        cvtColor(frame, gray_frame, COLOR_BGR2GRAY);
        imshow("Decrypted Video", gray_frame);

        if (waitKey(30) >= 0) {
            break;
        }
    }

    cap.release();
    destroyAllWindows();
}

// Function to reset video
void reset_fun(string filename) {
    VideoCapture cap(filename);

    if (!cap.isOpened()) {
        cout << "Error opening video stream or file" << endl;
        return;
    }

    namedWindow("Original Video", WINDOW_AUTOSIZE);
    while (true) {
        Mat frame;
        bool success = cap.read(frame);

        if (!success) {
            cout << "Error capturing frame from video" << endl;
            break;
        }

        imshow("Original Video", frame);

        if (waitKey(30) >= 0) {
            break;
        }
    }

    cap.release();
    destroyAllWindows();
}

int main() 
{
    Fl::run();
    return 0;
}
