/*
    Copyright 2015-2020 Clément Gallet <clement.gallet@ens-lyon.org>

    This file is part of libTAS.

    libTAS is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    libTAS is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with libTAS.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "sockethelpers.h"
#include <sys/socket.h>
#include <sys/stat.h>
#include <cstdlib>
#include <unistd.h>
#include <sys/un.h>
#include <iostream>
#include <vector>

#ifdef SOCKET_LOG
#include "lcf.h"
#include "../library/logging.h"
#else
#include <iostream>
#endif

#define SOCKET_FILENAME "/tmp/libTAS.socket"

/* Socket to communicate between the program and the game */
static int socket_fd = 0;

void removeSocket(void){
    unlink(SOCKET_FILENAME);
}

bool initSocketProgram(void)
{
    const struct sockaddr_un addr = { AF_UNIX, SOCKET_FILENAME };
    socket_fd = socket(AF_UNIX, SOCK_STREAM, 0);

    struct timespec tim = {0, 500L*1000L*1000L};

    const int MAX_RETRIES = 10;
    int retry = 0;

    nanosleep(&tim, NULL);
    while (connect(socket_fd, reinterpret_cast<const struct sockaddr*>(&addr),
                sizeof(struct sockaddr_un))) {
        std::cout << "Attempt " << retry + 1 << ": Couldn't connect to socket." << std::endl;
        retry++;
        if (retry < MAX_RETRIES) {
            nanosleep(&tim, NULL);
        } else {
            return false;
        }
    }
    std::cout << "Attempt " << retry + 1 << ": Connected." << std::endl;

    return true;
}

bool initSocketGame(void)
{
    /* Check if socket file already exists. If so, it is probably because
     * the link is already done in another process of the game.
     * In this case, we just return immediately.
     */
    struct stat st;
    int result = stat(SOCKET_FILENAME, &st);
    if (result == 0)
        return false;

    const struct sockaddr_un addr = { AF_UNIX, SOCKET_FILENAME };
    const int tmp_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (bind(tmp_fd, reinterpret_cast<const struct sockaddr*>(&addr), sizeof(struct sockaddr_un)))
    {
        std::cerr << "Couldn't bind client socket." << std::endl;
        exit(-1);
    }

    if (listen(tmp_fd, 1))
    {
        std::cerr << "Couldn't listen on client socket." << std::endl;
        exit(-1);
    }

    if ((socket_fd = accept(tmp_fd, NULL, NULL)) < 0)
    {
        std::cerr << "Couldn't accept client connection." << std::endl;
        exit(-1);
    }

    close(tmp_fd);
    return true;
}

void closeSocket(void)
{
    close(socket_fd);
}

void sendData(const void* elem, unsigned int size)
{
#ifdef SOCKET_LOG
    libtas::debuglogstdio(LCF_SOCKET, "Send socket data of size %u", size);
#endif

    ssize_t ret = 0;
    do {
        ret = send(socket_fd, elem, size, 0);
    } while ((ret == -1) && (errno == EINTR));

    if (ret == -1) {
#ifdef SOCKET_LOG
        libtas::debuglogstdio(LCF_SOCKET | LCF_ERROR, "send() returns -1 with error %s", strerror(errno));
#else
        std::cerr << "send() returns -1 with error " << strerror(errno) << std::endl;
#endif
    }
    else if (ret != size) {
#ifdef SOCKET_LOG
        libtas::debuglogstdio(LCF_SOCKET | LCF_ERROR, "send() %u bytes instead of %u", ret, size);
#else
        std::cerr << "send() " << ret << " bytes instead of " << size << std::endl;
#endif
    }
}

void sendMessage(int message)
{
#ifdef SOCKET_LOG
    libtas::debuglogstdio(LCF_SOCKET, "Send socket message %d", message);
#endif
    sendData(&message, sizeof(int));
}

void sendString(const std::string& str)
{
#ifdef SOCKET_LOG
    libtas::debuglog(LCF_SOCKET, "Send socket string ", str);
#endif
    unsigned int str_size = str.size();
    sendData(&str_size, sizeof(unsigned int));
    sendData(str.c_str(), str_size);
}

int receiveData(void* elem, unsigned int size)
{
#ifdef SOCKET_LOG
    libtas::debuglogstdio(LCF_SOCKET, "Receive socket data of size %u", size);
#endif

    ssize_t ret = 0;
    do {
        ret = recv(socket_fd, elem, size, MSG_WAITALL);
    } while ((ret == -1) && (errno == EINTR));

    if (ret == -1) {
#ifdef SOCKET_LOG
        libtas::debuglogstdio(LCF_SOCKET | LCF_ERROR, "recv() returns -1 with error %s", strerror(errno));
#else
        std::cerr << "recv() returns -1 with error " << strerror(errno) << std::endl;
#endif
    }
    else if (ret != size) {
#ifdef SOCKET_LOG
        libtas::debuglogstdio(LCF_SOCKET | LCF_ERROR, "recv() %u bytes instead of %u", ret, size);
#else
        std::cerr << "recv() " << ret << " bytes instead of " << size << std::endl;
#endif
    }
    return ret;
}

int receiveMessage()
{
    int msg;
    int ret = receiveData(&msg, sizeof(int));
#ifdef SOCKET_LOG
    libtas::debuglogstdio(LCF_SOCKET, "Receive socket message %d", ret);
#endif

    if (ret < 0)
        return ret;
    return msg;
}

int receiveMessageNonBlocking()
{
    int msg;
    int ret = recv(socket_fd, &msg, sizeof(int), MSG_WAITALL | MSG_DONTWAIT);
    if (ret < 0)
        return ret;
#ifdef SOCKET_LOG
    libtas::debuglogstdio(LCF_SOCKET, "Receive non-blocking socket message %d", msg);
#endif
    return msg;
}

std::string receiveString()
{
    unsigned int str_size;
    receiveData(&str_size, sizeof(unsigned int));

    /* TODO: There might be a better way to do this...? */
    std::vector<char> buf(str_size, 0x00);
    receiveData(buf.data(), str_size);

    std::string str(buf.data(), str_size);
#ifdef SOCKET_LOG
    libtas::debuglog(LCF_SOCKET, "Receive socket string ", str);
#endif
    return str;
}

void receiveCString(char* str)
{
    unsigned int str_size;
    receiveData(&str_size, sizeof(unsigned int));
    receiveData(str, str_size);
    str[str_size] = '\0';
#ifdef SOCKET_LOG
    libtas::debuglogstdio(LCF_SOCKET, "Receive socket C string %s", str);
#endif
}
