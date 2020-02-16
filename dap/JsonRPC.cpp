#include "Exception.hpp"
#include "JsonRPC.hpp"
#include "SocketServer.hpp"
#include "StringUtils.hpp"
#include <iostream>

dap::JsonRPC::JsonRPC() {}

dap::JsonRPC::~JsonRPC() {}

dap::ProtocolMessage::Ptr_t dap::JsonRPC::ProcessBuffer()
{
    if(m_buffer.empty()) {
        return nullptr;
    }
    // Find the "Content-Length:" string
    unordered_map<string, string> headers;
    int headerSize = ReadHeaders(headers);
    if(headerSize == -1) {
        return nullptr;
    }
    // We got the headers, check to see that we have the "Content-Length" one
    auto iter = headers.find("Content-Length");
    if(iter == headers.end()) {
        // this is a problem in the protocol. If we restore this section back to the buffer
        // we will simply stuck with it again later. So we remove it and return null
        m_buffer.erase(headerSize);
        cerr << "ERROR: Read complete header section. But no Content-Length header was found" << endl;
        return nullptr;
    }

    string contentLength = iter->second;
    long msglen = std::atol(contentLength.c_str());
    if(msglen <= 0) {
        cerr << "ERROR: Invalid Content-Length header value: 0 or lower than 0" << endl;
        return nullptr;
    }

    long buflen = m_buffer.length();
    if((headerSize + msglen) > buflen) {
        // not enough buffer
        return nullptr;
    }

    // Read the payload into a separate buffer and remove the full message
    // from the m_buffer member
    string payload(m_buffer.begin() + headerSize, m_buffer.begin() + headerSize + msglen);
    m_buffer.erase(0, headerSize + msglen);

    JSON json = JSON::Parse(payload);
    if(!json.IsOK()) {
        return nullptr;
    }
    string type = json["type"].GetString();
    string command = (type == "event") ? json["event"].GetString() : json["command"].GetString();

    // Allocate
    ProtocolMessage::Ptr_t message = ObjGenerator::Get().New(type, command);
    if(!message) {
        return nullptr;
    }
    message->From(json);
    return message;
}

int dap::JsonRPC::ReadHeaders(unordered_map<string, string>& headers)
{
    size_t where = m_buffer.find("\r\n\r\n");
    if(where == string::npos) {
        return -1;
    }
    string headerSection = m_buffer.substr(0, where); // excluding the "\r\n\r\n"
    vector<string> lines = StringUtils::Split(headerSection, '\n');
    for(string& header : lines) {
        StringUtils::Trim(header);
        string name = StringUtils::BeforeFirst(header, ':');
        string value = StringUtils::AfterFirst(header, ':');
        headers.insert({ StringUtils::Trim(name), StringUtils::Trim(value) });
    }
    // return the headers section + the separator
    return (where + 4);
}

void dap::JsonRPC::SetBuffer(const string& buffer) { m_buffer = buffer; }

void dap::JsonRPC::AppendBuffer(const string& buffer) { m_buffer.append(buffer); }

void dap::JsonRPC::Send(ProtocolMessage& msg, SocketBase::Ptr_t conn) const
{
    if(!conn) {
        throw Exception("Invalid connection");
    }
    string network_buffer;
    string payload = msg.ToString();
    network_buffer = "Content-Length: ";
    network_buffer += to_string(payload.length());
    network_buffer += "\r\n\r\n";
    network_buffer += payload;
    conn->Send(network_buffer);
}

void dap::JsonRPC::Send(ProtocolMessage::Ptr_t msg, SocketBase::Ptr_t conn) const
{
    if(!msg) {
        throw Exception("Unable to send empty message");
    }
    if(!conn) {
        throw Exception("Invalid connection");
    }
    Send(*msg.get(), conn);
}
