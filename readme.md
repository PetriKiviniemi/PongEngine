PongEngine is the classic game of pong turned into a cloud game.

The project has server/client architecture.  
The videoframes are streamed to the client from the server,  
and the user input is streamed to the server from the client.  

The videostreaming logic is as of following:   
1. Server draws the screen content   
2. Server takes the raw pixel data and encodes it into MPEG4 format   
3. The frame is split into fragments   
4. The fragments are sent to the client via UDP and IPv4   
5. The client collects the fragments, parses AVPackets from the bytes   
6. The AVPacket is decoded into a frame   
7. The AVFrame is drawn into a texture and rendered on screen  
  
The user input is more simple, the keystrokes are simply sent over UDP from client to server   

Future optimizations:  
Place decoded frames into larger buffers/batches to reduce the queue access count (Mutexes are causing a lot of overhead).  
Place the userinput into larger buffers for same reason.  
