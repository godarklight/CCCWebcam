#include <iostream>
#include <string>
#include <memory>
#include <curl/curl.h>
#include <opencv2/opencv.hpp>
#include <zbar.h>

//Webcam Stuff
const std::string WINDOW_NAME = "CCCWebcam";
std::unique_ptr<cv::VideoCapture> vc;

//HTTP Stuff
std::string httpDestination = "http://godarklight.info.tm/webcam/index.php";
std::string lastID = "(none)";

//Scanner stuff
int frameNumber = 0;
const int SCAN_FRAMES = 15;
zbar::ImageScanner scanner;

void setup_command_line(int argc, char** argv)
{
    if (argc == 2)
    {
        httpDestination = std::string(argv[1]);
    }
    std::cout << "Using " << httpDestination << " as the webcam server" << std::endl;
}

int setup_zbar()
{
    scanner.set_config(zbar::ZBAR_NONE, zbar::ZBAR_CFG_ENABLE, 1);
    return 1;
}

int setup_webcam()
{
    cv::namedWindow(WINDOW_NAME, CV_WINDOW_AUTOSIZE);
    int ok = 1;
    vc = std::unique_ptr<cv::VideoCapture>(new cv::VideoCapture(0));
    if (!vc->isOpened())
    {
        ok = 0;
        std::cout << "Error while setting up webcam" << std::endl;
    }
    for (int i = 0; i < 30; i++)
    {
        cv::Mat mat;
        vc->read(mat);
        imshow(WINDOW_NAME, mat);
    }
    return ok;
}

int setup_curl()
{
    int ok = 1;
    curl_global_init(CURL_GLOBAL_DEFAULT);
    CURL *curl = curl_easy_init();
    if (!curl)
    {
        ok = 0;
        std::cout << "Error while setting up cURL" << std::endl;
    }
    curl_easy_cleanup(curl);
    return ok;
}

int send_webcam_id(std::string id)
{
    int ok = 1;
    CURL* curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_URL, httpDestination.c_str());
    char* idEncoded = curl_easy_escape(curl, id.c_str(), id.length());
    std::string postData = "id=" + std::string(idEncoded);
    curl_free(idEncoded);
    curl_easy_setopt(curl, CURLOPT_POST, 1);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postData.c_str());
    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK)
    {
        ok = 0;
        std::cout << "HTTP Error: " << curl_easy_strerror(res) << std::endl;
    }
    else
    {
        std::cout << "HTTP OK: '" << id << "'" << std::endl;
    }
    curl_easy_cleanup(curl);
    return ok;
}

void scan_image(cv::Mat camImage)
{
    std::cout << "Scanning..." << std::endl;
    cv::Mat grayImage;
    cv::cvtColor(camImage, grayImage, CV_BGR2GRAY);
    int width = grayImage.cols;
    int height = grayImage.rows;
    uchar* raw = grayImage.data;
    zbar::Image zImage(width, height, "Y800", raw, width*height);
    scanner.scan(zImage);
    for (auto it = zImage.symbol_begin(); it != zImage.symbol_end(); ++it)
    {
        std::string scannedID = it->get_data();
        if (lastID != scannedID)
        {
            lastID = scannedID;
            send_webcam_id(scannedID);
        }
    }
}

int webcam_loop()
{
    int ok = 1;
    cv::Mat camImage;
    if (!vc->read(camImage))
    {
        std::cout << "Webcam Read Error" << std::endl;
        ok = -1;
    }
    else
    {
        frameNumber++;
        if (frameNumber > SCAN_FRAMES)
        {
            scan_image(camImage);
            frameNumber = 0;
        }
        cv::putText(camImage, "Last scan: " + lastID, cv::Point(30,30), cv::FONT_HERSHEY_PLAIN, 1, cv::Scalar::all(255), 1, 8);
        cv::imshow(WINDOW_NAME, camImage);
        cv::waitKey(1);
    }
    return ok;
}

int main(int argc, char** argv)
{
    setup_command_line(argc, argv);
    if (!setup_zbar())
    {
        return -1;
    }
    if(!setup_webcam())
    {
        return -2;
    }
    if (!setup_curl())
    {
        return -3;
    }
    while(webcam_loop())
    {
    }
    return 0;
}
