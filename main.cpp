#include <iostream>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

class Session : public std::enable_shared_from_this<Session> {
public:
    Session(tcp::socket socket) : socket_(std::move(socket)) {}

    void start() {
        do_read();
    }

private:
    void do_read() {
        auto self(shared_from_this());
        socket_.async_read_some(boost::asio::buffer(data_, max_length),
            [this, self](boost::system::error_code ec, std::size_t length) {
                if (!ec) {
                    // Store received data in a persistent string
                    request_data_.append(data_, length);
                    
                    // Check if we've received a complete HTTP request
                    if (request_data_.find("\r\n\r\n") != std::string::npos) {
                        do_write();
                    } else {
                        // Continue reading if request isn't complete
                        do_read();
                    }
                }
            });
    }

    void do_write() {
        auto self(shared_from_this());
        
        // Create HTTP response
        std::string response = 
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/plain\r\n"
            "Content-Length: 13\r\n"
            "Connection: close\r\n"
            "\r\n"
            "Hello, World!";
            
        boost::asio::async_write(socket_, boost::asio::buffer(response),
            [this, self](boost::system::error_code ec, std::size_t /*length*/) {
                if (!ec) {
                    // Close connection after response
                    socket_.close();
                }
            });
    }

    tcp::socket socket_;
    enum { max_length = 1024 };
    char data_[max_length];
    std::string request_data_; // Persistent storage for request data
};

class Server {
public:
    Server(boost::asio::io_context& io_context, short port)
        : acceptor_(io_context, tcp::endpoint(tcp::v4(), port)) {
        do_accept();
    }

private:
    void do_accept() {
        acceptor_.async_accept(
            [this](boost::system::error_code ec, tcp::socket socket) {
                if (!ec) {
                    std::make_shared<Session>(std::move(socket))->start();
                }
                do_accept();
            });
    }

    tcp::acceptor acceptor_;
};

#include <csignal>
#include <atomic>

std::atomic<bool> running{true};

void handle_signal(int) {
    running.store(false);
}

int main(int argc, char* argv[]) {
    try {
        if (argc != 2) {
            std::cerr << "Usage: cpp_server <port>\n";
            return 1;
        }

        boost::asio::io_context io_context;
        boost::asio::signal_set signals(io_context, SIGINT, SIGTERM);
        signals.async_wait([&](auto, auto) {
            io_context.stop();
        });

        Server s(io_context, std::atoi(argv[1]));

        std::thread t([&io_context]() {
            io_context.run();
        });

        std::signal(SIGINT, handle_signal);
        std::signal(SIGTERM, handle_signal);

        std::cout << "Server running on port " << argv[1] << "\n";
        while (running) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        io_context.stop();
        if (t.joinable()) {
            t.join();
        }

        std::cout << "Server shutdown complete\n";
    }
    catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
