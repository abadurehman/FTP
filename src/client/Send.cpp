#include "./client.hpp"

// Send functions

/**Send : wrapper for system call `send`
 *
 * @usage
 * sends the given string terminated by FTP::DELIM
 *
 * @returns
 * no of bytes sent
*/
int Client::Send(int sockfd, const string& msgToSend, int responseCode) {
	string msg;
	if (responseCode != 0) {
		msg += string("[Request : ") + to_string(responseCode) + string("] \n");
	}
	msg += msgToSend;
	msg += FTP::DELIM;
	// logs(msg.c_str());
	return _send_all(sockfd, msg.c_str(), msg.size());
}

/**_send_all : Internal function
 *
 * @usage
 * Used to send entire Buffer given in arguments
 *
 * @returns
 * total no of bytes sent.
 *
*/
int Client::_send_all(int sockfd, const void* buffer, size_t length) {
	size_t totalNumberBytesSent = 0;
	while (totalNumberBytesSent < length) {
		int noOfBytesSent =
			send(sockfd, buffer, length - totalNumberBytesSent, MSG_NOSIGNAL);
		if (noOfBytesSent == -1) {
			printError("[SEND]");
			break;
		}
		totalNumberBytesSent += noOfBytesSent;
	}

	if (totalNumberBytesSent < length) {
		return -1;
	}

	return totalNumberBytesSent;
}

/**SendFile : Wrapper function : sends a file across
 *
 * @usage
 * Send a file specified by filename, in binary format
 *
 * @returns
 * total no of bytes sent.
 *
*/
int Client::SendFile(int sockfd, const string& filename) {
	struct stat st;
	int statcode = stat(filename.c_str(), &st);
	if (statcode == -1) {
		return -1;
	}

	int size = st.st_size;

	FILE* file;
	file = fopen(filename.c_str(), "rb");
	size_t noOfBytesSent = _send_all_binary(sockfd, file, size);
	printInfo("Bytes Sent : ", noOfBytesSent);
	fclose(file);

	return 0;
}

/**_send_all_binary : Internal function
 *
 * @usage
 * Send the entire file (pointed by fd)
 *
 * @returns
 * total no of bytes sent.
 *
*/
int Client::_send_all_binary(int sockfd, FILE* fd, int size) {
	int sizeOfFile = size;
	unsigned char buffer[FTP::FILE_READ_BUFFER_SIZE + 1];

	int totalNoOfBytesSent = 0;
	int noOfBytesSent = 0;
	int noOfBytesRead = 0;
	while (sizeOfFile > 0) {
		noOfBytesRead = fread(buffer, 1, FTP::FILE_READ_BUFFER_SIZE, fd);
		noOfBytesSent = _send_all(sockfd, buffer, noOfBytesRead);
		if (noOfBytesSent == -1) {
			printError("[CLIENT:SEND:BINARY]");
			break;
		}
		sizeOfFile -= noOfBytesRead;
		totalNoOfBytesSent += noOfBytesSent;
	}
	if (sizeOfFile > 0) {
		return -1;
	}
	return totalNoOfBytesSent;
}
