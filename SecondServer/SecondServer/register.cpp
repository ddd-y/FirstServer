#include"register.h"



Register::Register(const std::string& ip_, const short port_) : first_server_ip(ip_),first_server_port(port_)
{	
	
	
	memset(buffer, 0, sizeof(buffer));
}


Register::~Register()
{	
	if(fd != -1) close(fd);
}

short Register::start()
{	
	fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd == -1)
	{
		std::cerr << "Failed to get fd" << std::endl;
		return -1;
	}

	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(first_server_port);
	inet_pton(AF_INET, first_server_ip.c_str(), &saddr.sin_addr);

	int ret = connect(fd, (const sockaddr*)&saddr, sizeof(saddr));
	if (ret == -1)
	{	
		std::cerr << "Failded to connect to the server" << std::endl;
		return -1;
	}
	else
	{
		std::cout << "connect" << std::endl;
	}
	//³¢ÊÔ×¢²á
	const std::string message = "Request";
	snprintf(buffer, sizeof(buffer), "%s", message.c_str());
	send(fd, buffer, strlen(buffer) + 1, 0);
	
	memset(buffer, 0, sizeof(buffer));
	ssize_t len = recv(fd, buffer, sizeof(buffer), 0);
	std::cout << len << std::endl;


	if (len > 0)
	{	
		std::cout << "server say:" << buffer << std::endl;

	}
	else if (len == 0)
	{
		std::cout << "server close!" << std::endl;
	}
	else
	{
		std::cerr << "Failed to recv the message" << std::endl;
	}

	return 1;

}

void Register::heart_info_to_server()
{




}


