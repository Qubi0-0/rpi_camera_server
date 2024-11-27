#include "../lib/httplib.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <mutex>
#include <string>
#include <vector>
#include <filesystem>
#include <thread>
#include <chrono>

// Mutex to ensure safe access to the frame
std::mutex frameMutex;
std::vector<unsigned char> currentFrame; // Store the current frame data as JPEG

// Function to load the image into currentFrame
bool loadImage(const std::string &path)
{
    std::ifstream file(path, std::ios::binary);
    if (file)
    {
        std::vector<unsigned char> buffer((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        std::lock_guard<std::mutex> lock(frameMutex);
        currentFrame = std::move(buffer);
        return true;
    }
    else
    {
        std::cerr << "Error: Unable to load image from " << path << std::endl;
        return false;
    }
}

// Function to serve HTML files
void serveHTML(const std::string &filePath, const httplib::Request &, httplib::Response &res)
{
    std::ifstream htmlFile(filePath);
    if (htmlFile)
    {
        std::stringstream buffer;
        buffer << htmlFile.rdbuf();
        res.set_content(buffer.str(), "text/html");
    }
    else
    {
        std::cerr << "Error: " << filePath << " not found" << std::endl;
        res.status = 404;
        res.set_content("File not found", "text/plain");
    }
}

// Function to serve the latest frame (frame.jpg)
void serveFrame(const std::string &filePath, const httplib::Request &, httplib::Response &res)
{
    std::ifstream file(filePath, std::ios::binary);
    if (file)
    {
        std::vector<unsigned char> buffer((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        res.set_content(reinterpret_cast<const char *>(buffer.data()), buffer.size(), "image/jpeg");
    }
    else
    {
        std::cerr << "Error: Frame not available at " << filePath << std::endl;
        res.status = 404;
        res.set_content("Frame not available", "text/plain");
    }
}

// Function to serve CSS files
void serveCSS(const std::string &filePath, const httplib::Request &, httplib::Response &res)
{
    std::ifstream cssFile(filePath);
    if (cssFile)
    {
        std::stringstream buffer;
        buffer << cssFile.rdbuf();
        res.set_content(buffer.str(), "text/css");
    }
    else
    {
        std::cerr << "Error: CSS file not found" << std::endl;
        res.status = 404;
        res.set_content("CSS file not found", "text/plain");
    }
}

// Function to serve static files (GIFs, audio, etc.)
void serveStatic(const std::string &filePath, const std::string &contentType, const httplib::Request &, httplib::Response &res)
{
    std::ifstream file(filePath, std::ios::binary);
    if (file)
    {
        std::stringstream buffer;
        buffer << file.rdbuf();
        res.set_content(buffer.str(), contentType.c_str());
    }
    else
    {
        std::cerr << "Error: " << filePath << " not found" << std::endl;
        res.status = 404;
        res.set_content("File not found", "text/plain");
    }
}

int main()
{
    httplib::Server server;

    // Serve the index.html when accessing the root ("/")
    server.Get("/", [](const httplib::Request &req, httplib::Response &res)
               { serveHTML("../index.html", req, res); });

    // Serve the mlg.html when accessing "/mlg.html"
    server.Get("/mlg.html", [](const httplib::Request &req, httplib::Response &res)
               { serveHTML("../mlg.html", req, res); });

    // Serve the CSS file ("/css/style.css")
    server.Get("/css/style.css", [](const httplib::Request &req, httplib::Response &res)
               { serveCSS("../css/style.css", req, res); });

    // Serve the mlg.css file ("/css/mlg.css")
    server.Get("/css/mlg.css", [](const httplib::Request &req, httplib::Response &res)
               { serveCSS("../css/mlg.css", req, res); });

    // Serve GIF files
    server.Get("/gifs/(.*)", [](const httplib::Request &req, httplib::Response &res)
               {
        std::string filePath = "../gifs/" + req.matches[1].str();
        serveStatic(filePath, "image/gif", req, res); });

    // Serve audio files
    server.Get("/audio/(.*)", [](const httplib::Request &req, httplib::Response &res)
               {
        std::string filePath = "../audio/" + req.matches[1].str();
        serveStatic(filePath, "audio/mpeg", req, res); });

    // Serve frame images
    server.Get("/frames/(.*)", [](const httplib::Request &req, httplib::Response &res)
               {
        std::string filePath = "../frames/" + req.matches[1].str();
        serveFrame(filePath, req, res); });

    std::cout << "Server running at http://localhost:8080" << std::endl;
    if (!server.listen("0.0.0.0", 8080))
    {
        std::cerr << "Error: Failed to start server" << std::endl;
        return 1;
    }

    return 0;
}