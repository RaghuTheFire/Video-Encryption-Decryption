
// Include necessary libraries
#include <QApplication>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QLabel>
#include <QTextEdit>
#include <QPushButton>
#include <QFileDialog>
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
void encrypt_fun(QTextEdit* path_text);
void decrypt_fun(string filename);
void reset_fun(string filename);

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr) : QMainWindow(parent)
    {
        // Set window properties
        setWindowTitle("Video Encryption Decryption");
        setFixedSize(1000, 700);

        // Create central widget and layout
        QWidget *centralWidget = new QWidget(this);
        QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);

        // Top label
        QLabel *start1 = new QLabel("VIDEO  ENCRYPTION\nDECRYPTION");
        start1->setFont(QFont("Times", 55, QFont::Bold | QFont::ItalicStyle));
        start1->setStyleSheet("color: magenta;");
        mainLayout->addWidget(start1, 0, Qt::AlignCenter);

        // Selected video label
        QLabel *lbl2 = new QLabel("Selected Video");
        lbl2->setFont(QFont("Times", 30, QFont::Bold));
        lbl2->setStyleSheet("color: brown;");
        mainLayout->addWidget(lbl2, 0, Qt::AlignLeft);

        // Text editor for displaying selected video path
        path_text = new QTextEdit;
        path_text->setFont(QFont("Times", 30, QFont::Bold));
        path_text->setStyleSheet("color: orange; background-color: yellow;");
        mainLayout->addWidget(path_text);

        // Encrypt video button
        QPushButton *encrypt_button = new QPushButton("ENCRYPT VIDEO");
        encrypt_button->setFont(QFont("Times", 25, QFont::Bold));
        encrypt_button->setStyleSheet("color: blue; background-color: orange;");
        connect(encrypt_button, &QPushButton::clicked, this, [this]() { encrypt_fun(path_text); });
        mainLayout->addWidget(encrypt_button, 0, Qt::AlignCenter);

        // Decrypt video button
        QPushButton *decrypt_button = new QPushButton("DECRYPT VIDEO");
        decrypt_button->setFont(QFont("Times", 25, QFont::Bold));
        decrypt_button->setStyleSheet("color: blue; background-color: orange;");
        connect(decrypt_button, &QPushButton::clicked, this, [this]() { decrypt_fun(filename); });
        mainLayout->addWidget(decrypt_button, 0, Qt::AlignCenter);

        // Select video button
        QPushButton *select_button = new QPushButton("SELECT");
        select_button->setFont(QFont("Times", 25, QFont::Bold));
        select_button->setStyleSheet("color: blue; background-color: green;");
        connect(select_button, &QPushButton::clicked, this, [this]() {
            filename = QFileDialog::getOpenFileName(this, "Select Video File", "", "Video Files (*.mp4)");
            path_text->setText(filename);
        });
        mainLayout->addWidget(select_button, 0, Qt::AlignLeft);

        // Reset video button
        QPushButton *reset_button = new QPushButton("RESET");
        reset_button->setFont(QFont("Times", 25, QFont::Bold));
        reset_button->setStyleSheet("color: blue; background-color: yellow;");
        connect(reset_button, &QPushButton::clicked, this, [this]() { reset_fun(filename); });
        mainLayout->addWidget(reset_button, 0, Qt::AlignHCenter);

        // Exit button
        QPushButton *exit_button = new QPushButton("EXIT");
        exit_button->setFont(QFont("Times", 25, QFont::Bold));
        exit_button->setStyleSheet("color: blue; background-color: red;");
        connect(exit_button, &QPushButton::clicked, this, &QApplication::quit);
        mainLayout->addWidget(exit_button, 0, Qt::AlignRight);

        setCentralWidget(centralWidget);
    }

private:
    QTextEdit *path_text;
};

// Function to encrypt video
void encrypt_fun(QTextEdit* path_text) {
    filename = path_text->toPlainText().toStdString();

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

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    MainWindow window;
    window.show();
    return app.exec();
}
