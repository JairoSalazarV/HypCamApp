#include "lstStructs.h"
#include "hypCamAPI.h"

#include <unistd.h>
#include <netdb.h>
#include <QFile>
#include <arpa/inet.h>
#include <math.h>

#include <QDebug>
#include <qrgb.h>
#include <rasphypcam.h>

//#include <QtCore>
//#include <QtNetwork/QtNetwork>



void funcValCam( std::string IP, int portno, camSettings *tmpCamSett, QWidget* parent)
{
    int sockfd, n, tmpFrameLen;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    //funcShowMsg("IP: ", QString(IP.c_str()));
    if (sockfd < 0){
        funcShowMsgERROR_Timeout("opening socket",parent);
    }else{
        server = gethostbyname( IP.c_str() );
        if (server == NULL)
        {
            funcShowMsgERROR_Timeout("No such host",parent);
            //funcShowMsgERROR("no such host");
        }else{
            bzero((char *) &serv_addr, sizeof(serv_addr));
            serv_addr.sin_family = AF_INET;
            bcopy((char *)server->h_addr,
                (char *)&serv_addr.sin_addr.s_addr,
                server->h_length);
            serv_addr.sin_port = htons(portno);
            if (::connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
            {
                funcShowMsgERROR_Timeout("Connecting",parent);
                //funcShowMsgERROR("connecting");
            }else{
                //Request camera settings
                char tmpIdMsg = (char)1;
                tmpFrameLen = sizeof(camSettings);
                unsigned char bufferRead[tmpFrameLen];
                n = ::write(sockfd,&tmpIdMsg,1);
                if (n < 0)
                {
                    funcShowMsgERROR_Timeout("Writing to socket",parent);
                    //funcShowMsgERROR("writing to socket");
                }else{
                    //Receibing ack with file len
                    n = read(sockfd,bufferRead,tmpFrameLen);
                    if (n < 0)
                    {
                        funcShowMsgERROR_Timeout("Reading socket",parent);
                        //funcShowMsgERROR("reading socket");
                    }else{
                        memcpy(tmpCamSett,&bufferRead,tmpFrameLen);
                    }
                }
            }
        }
        ::close(sockfd);
    }
}


