[![Actions Status](https://github.com/ZenConn/Server/workflows/Ubuntu/badge.svg)](https://github.com/ZenConn/Server/actions)
[![Actions Status](https://github.com/ZenConn/Server/workflows/Style/badge.svg)](https://github.com/ZenConn/Server/actions)
[![codecov](https://codecov.io/gh/ZenConn/Server/branch/master/graph/badge.svg)](https://codecov.io/gh/ZenConn/Server)

<p align="center">
  <img src="https://avatars.githubusercontent.com/u/159615387?s=400&v=4" height="175" width="auto" />
</p>

  <h1 align="center">Server</h1>


## Features

### HTTP and Websockets

Server is capable of distributing static content from the `public` folder and establishing persistent connections, without encryption or compression routines.

## Next

Consider the following as a todo list.

### Sessions

WebSockets connections must be recorded in database and linked to the server.

### Logging

Something like nginx or apache format.

### Authentication / Authorization

Someone must be able to be authenticated and granted.

### Broadcasting

Someone with grants, must be able to dispatch a message to all but not himself.

### Public and private channels

Someone with and without grants, must be able to join to private or public channels.

### Channels broadcasting

Someone with or without grants, must be able to broadcast into public or/and private channels.

### Process communication

Instances can be agent or worker

Servers running as agent should be able to run, synchronize child processes as workers.

Worker process must notify when a new connection is established while agent should push the connection into a list when is notified.

### Distributed

> While agent is responsible for workers and must have known who are and what are doing,
> 
> Workers are responsible for keeping client connections opened to receive and distribute messages.

Someone must be able to connect into a worker or agent node and broadcast the message to the subscribed clients.

Someone must be able to connect into a worker or agent and get authorized and subscribed to a private channel but a new connection notification must be sent.

### Dockerized

Someone interested can deploy the system by 

```shell
docker compose up
```

### JSON and Binary protocol

Someone should be able to send headers like `X-Bin-Protocol` and `X-JSON-Protocol` to reduce the response size.

### Data Persistence

Someone should be able to audit the system. It includes all the messages produced by the system.

### Performance

Someone should be able to run the program, but it should not produce any kind of performance issue or memory leak.

### Secured

Someone should be able to add a new layer to deal with SSL stuffs.

## How to contribute

It's a good start point:

> The goal is move those Next into the Features. 
> 
> Q: How hard it's that?
> 
> A: It's like move a giant rock with the mind.

## License

> The software, code or repository, in any of its forms, adopted, present and future, is provided as is, which means that there is no guarantee of any kind, while permissive to carry out any type of development, whether technological or code , business or educational (exclusive), the owners of the code and even those interested or not interested, will be authorized to copy, modify and reproduce the code, publicly or privately, only after reading the license, as well as the obligation to render tribute, that is, the inclusion of this license in all distributions and execution instances of the program. In the event of any dispute or discrepancy, in any of the points and symbols herein, all rights and permissions will be revoked.