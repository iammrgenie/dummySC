#include <iostream>
#include <chrono>
#include <SEAL-4.0/seal/seal.h>

#define ASIO_STANDALONE

#include <asio.hpp>
#include <asio/ts/buffer.hpp>
#include <asio/ts/internet.hpp>

#include "sealhelper.h"

using namespace std;
using namespace asio;
using namespace seal;

string readData(ip::tcp::socket & socket) {
    asio::streambuf buf;
    asio::read_until( socket, buf, "\n" );
    string data = asio::buffer_cast<const char*>(buf.data());
    return data;
}

void sendData(ip::tcp::socket & socket, const string& message) {
    const string msg = message + "\n";
    asio::write( socket, asio::buffer(message) );
}

stringstream parms_stream;
stringstream data_stream;
stringstream sk_stream;
stringstream pk_stream;

int main() {
    
    EncryptionParameters parms(scheme_type::ckks);
    size_t poly_modulus_degree = 8192;
    parms.set_poly_modulus_degree(poly_modulus_degree);
    parms.set_coeff_modulus(CoeffModulus::Create(poly_modulus_degree, { 50, 30, 50 }));

    SEALContext context(parms);
    print_parameters(context);

    KeyGenerator keygen(context);
    SecretKey he_sk = keygen.secret_key();
    Serializable<PublicKey> he_pk = keygen.create_public_key();

    error_code ec;

    //Save Parameters
    auto parms_size = parms.save(parms_stream);
    string parms_string = parms_stream.str();

    //Save Public Key
    auto pk_size = he_pk.save(pk_stream);
    string pk_string = pk_stream.str();
    cout << "Serializable<PublicKeys>: wrote " << pk_size << " bytes" << endl;

    Serialization::SEALHeader header;
    Serialization::LoadHeader(pk_stream, header);

    cout << "Size indicated in SEALHeader: " << header.size << endl;
    cout << "SEALHeader magic: " << header.magic << endl;

    asio::io_service service_;
    
    //listen for new connection
    ip::tcp::acceptor acceptor_(service_, ip::tcp::endpoint(ip::tcp::v4(), 1500 ));

    //socket creation 
    ip::tcp::socket socket_(service_);
    
    cout << "[Server] Waiting for Client Connection ...\n";
    //waiting for connection
    acceptor_.accept(socket_);
    
    cout << "[Server] Client Connected. Waiting for incoming message ...\n";
    //read operation
    string message = readData(socket_);
    cout << "[Server]" << message << endl;
    
    //write operation parameters
    socket_.write_some(asio::buffer(parms_string.data(), parms_string.size()), ec);
    cout << "[Server] Paramters sent to Client!" << endl;

    //Receive Response
    string message2 = readData(socket_);
    cout << "[Server] " << message2 << endl;

    char key_length[10];
    sprintf(key_length, "%ld", pk_size);
    cout << "[Server] Size of Key = " << key_length << endl;

    //write operation public key
    socket_.write_some(asio::buffer(key_length, sizeof(key_length)), ec);
    cout << "[Server] Key size sent to Client!" << endl;

    //write operation public key
    // socket_.write_some(asio::buffer(pk_string.data(), pk_size), ec);
    // cout << "[Server] Public Key sent to Client!" << endl;

    // asio::write(socket_, asio::streambuf());
    asio::streambuf write_buffer;
    std::ostream output(&write_buffer);
    output << pk_string;
    auto bytes_transferred = asio::write(socket_, write_buffer);
    //cout << "Amount of Data transfer: " << bytes_transferred << endl;

    // auto bytes1_transferred = asio::write(socket_, write_buffer);

    // this_thread::sleep_for(2000ms);


    //socket_.wait(socket_.wait_write);

    // auto size_data = pk_string.size() / 2;
    // cout << "broken down size = " << size_data << endl;

    // for (int i = 0; i < 2; i++){
    //     char test[size_data];
    //     for (int j = 0; j < size_data; j++){
    //         test[j] = pk_string[j + (size_data * (i))];
    //     }


    // }

    return 0;
}