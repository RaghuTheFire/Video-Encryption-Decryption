# Video-Encryption-Decryption
This code uses the FLTK library for creating the GUI and OpenCV for video processing. 
The main window contains buttons for encrypting, decrypting, selecting a video file, resetting the video, and exiting the application. 
The `encrypt_fun` function converts the video to images, encrypts each image, and then creates an encrypted video from the encrypted images. 
The `decrypt_fun` function displays the video in grayscale, which serves as a simple decryption method. 
The `reset_fun` function plays the original video.
