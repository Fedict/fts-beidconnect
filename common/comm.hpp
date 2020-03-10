#ifndef __COMM_H
#define __COMM_H

#define E_COMM                               0xA001            //
 
#define E_COMM_PARAM						0xA0010011


//int ParseCL(int argc, char *argv[], COMMAND_PARAM *pCommand);
int readMessage(std::stringstream &stream);
int sendMessage(const std::string response);

#endif //__COMM_H

