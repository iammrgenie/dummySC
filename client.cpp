#include <iostream>
#include <SEAL-4.0/seal/seal.h>
#include "sealhelper.h"
#include <chrono>

#define ASIO_STANDALONE

#include <asio.hpp>
#include <asio/ts/buffer.hpp>
#include <asio/ts/internet.hpp>

using namespace std;
using namespace asio;
using namespace seal;

stringstream parms_stream;
stringstream data_stream;
stringstream sk_stream;
stringstream pk_stream;

// std::vector<char>pkBuffer(512000);

// void GrabData(ip::tcp::socket & socket){
//     socket.async_read_some(asio::buffer(pkBuffer.data(), pkBuffer.size()),
//     [&] (error_code ec, size_t len) {
//         if (!ec) {
//             cout << "Read " << len << " bytes \n";
//             for (int i = 0; i < len; i++)
//                 cout << pkBuffer[i];
            
//             GrabData(socket);
//         }
//     });
// }

std::string make_string(asio::streambuf& streambuf)
{
  return {asio::buffers_begin(streambuf.data()), 
          asio::buffers_end(streambuf.data())};
}

void readData(ip::tcp::socket & socket, error_code ec, string &inStr) {
    //string inStr;
    socket.wait(socket.wait_read);

    size_t bytes_available = socket.available();
    cout << "Bytes available: " << bytes_available << endl;

    if (bytes_available > 0){
        vector<char> vBuffer(bytes_available);
        socket.read_some(asio::buffer(vBuffer.data(), vBuffer.size()), ec);
        //cout << "[Client] Received: ";
        for (auto c : vBuffer) {
            //cout << c;
            inStr.push_back(c);
        }

    }
    //return inStr;
}

//Conversion to Char
void readKeyData(ip::tcp::socket & socket, error_code ec, string &inStr) {
    //string inStr;
    // socket.wait(socket.wait_read);

    size_t bytes_available = socket.available();
    cout << "Bytes available: " << bytes_available << endl;

    if (bytes_available > 0){
        vector<char> vBuffer(bytes_available);
        socket.read_some(asio::buffer(vBuffer.data(), vBuffer.size()), ec);
        // cout << "[Client] Received: ";
        for (auto c : vBuffer) {
            inStr.push_back(c);
        }
        //inStr = vBuffer.data();
        readKeyData(socket, ec, inStr);
    }
}

int main() {
    
    asio::io_service service_;
    
    //socket creation
    ip::tcp::socket socket_(service_);
    
    //Connection to Server
    socket_.connect( ip::tcp::endpoint( asio::ip::address::from_string("127.0.0.1"), 1500 ));

    string msg = "Hello from Client!\n";
    error_code ec;

    asio::write( socket_, asio::buffer(msg), ec );
    
    if( !ec ) {
        cout << "[Client] Client sent hello message!" << endl;
    } else {
        cout << "send failed: " << ec.message() << endl;
    }

    string parms_string; 
    readData(socket_, ec, parms_string);
    parms_stream << parms_string;

    EncryptionParameters parms;
    parms.load(parms_stream);
    print_parameters(parms);
            
    SEALContext context(parms);

    cout << "[Client] Acknowledging receipt of Parameters\n";
    string msg2 = "Parameters Received!\n";
    asio::write( socket_, asio::buffer(msg2), ec );

    //Receive size of incoming Key
    socket_.wait(socket_.wait_read);
    size_t bytes_available = socket_.available();
    long int pk_size;

    if (bytes_available > 0){
        vector<char> vBuffer(bytes_available);
        socket_.read_some(asio::buffer(vBuffer.data(), vBuffer.size()), ec);

        for (auto c : vBuffer) {
            //cout << c;
        }
        cout << endl;

        sscanf(vBuffer.data(), "%ld", &pk_size);
    }

    cout << "[Client] Incoming Key size: " << pk_size << endl;

    string pk_string;
    // socket_.wait(socket_.wait_read);
    
    // readKeyData(socket_, ec, pk_string);
    vector<char> buff(pk_size);
    
    while(1){
        int bytes_read = asio::read(socket_, asio::buffer(buff), ec);
        cout << "Size read: " << bytes_read << endl;
        if (bytes_read == 0)
            break;
    }
    // pk_stream << pk_string;

    // Serialization::SEALHeader header;
    // Serialization::LoadHeader(pk_stream, header);

    // cout << "Size indicated in SEALHeader: " << header.size << endl;
    // cout << "SEALHeader magic: " << header.magic << endl;
    // PublicKey pk;
    // pk.load(context, pk_stream);

    // socket_.wait(socket_.wait_read);
    // size_t bytes_avail = socket_.available();
    // cout << "Size of pending message: " << bytes_avail << endl;

    //vector<char> readBuff(pk_size);
    //string pk_string;
    // int i = 0;

    // while(1){
    //     int bytes_read = asio::read(socket_, asio::buffer(readBuff), ec);
    //     i++;
    //     cout << i << endl;
    //     copy(readBuff.begin(), readBuff.begin() + bytes_read, back_inserter(pk_string));
    //     if (bytes_read == 0)
    //         break;
    // }

    // cout << "Size of String = " << pk_string.size() << endl;
    //cout << "Received String = " << pk_string << endl;
    
    
    // size_t bytes_avail = 100000;
    // cout << bytes_avail << endl;
    // asio::streambuf readBuffer;
    // // cout << "1\n";
    // auto bytes_transferred = asio::read(socket_, readBuffer, asio::transfer_all());
    // cout << "Size of received message: " << bytes_transferred << endl;
    // cout << "2\n";
    // string pk_string = make_string(readBuffer);
    // cout << "Bytes = " << pk_string << endl;

    //Receive Public Key
    // string pk_string; 
    // readKeyData(socket_, ec, pk_size, pk_string);
    // // GrabData(socket_);
    // // this_thread::sleep_for(2000ms);

    // cout << "Size of String " << pk_string.size() << endl;
    // pk_stream << pk_string;

    // Serialization::SEALHeader header;
    // Serialization::LoadHeader(pk_stream, header);

    // cout << "Size indicated in SEALHeader: " << header.size << endl;
    // cout << "SEALHeader magic: " << header.magic << endl;
  
    // PublicKey pk;
    // cout << "\nTest 11\n";
    // pk.load(context, pk_stream);

    //}

        // // getting response from server
        // asio::streambuf receive_buffer;
        // asio::read(socket_, receive_buffer, asio::transfer_all(), ec);

        // if( ec && ec != asio::error::eof ) {
        //     cout << "receive failed: " << ec.message() << endl;
        // }
        // else {
        //     const char* data = asio::buffer_cast<const char*>(receive_buffer.data());
        //     cout << data << endl;
        //     for (int i = 0; i < sizeof(data); i ++) {
        //         cout << data[i];
        //     }
        // } 
    
      
    
    return 0;
}
