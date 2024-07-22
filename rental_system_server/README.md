# Overview
Established a high-performance reactor server, using a customized protocol on TCP/IP to provide user registration and login services. 

# Components
* Initializer: Loads configuration file, sets up log file, and initializes the service.
* Network Interface: This module based on libevent as the multiplexing internet interface.
* Customized Application Protocol: Adopting the session state to manage TCP/IP packet sticking and splitting, employs Protobuf for data serialization and deserialization.
* Thread Pool: Employs a thread pool to handle requests based on the subscriber and publisher design pattern, as well as to handle SQL database access tasks.
