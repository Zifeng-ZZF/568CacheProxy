# cache proxy
We built a proxy server which supports HTTP/HTTPS request from browser. It receives and sends request to the destination server then fetches response and returns to the browser. The communication is based on Socket (TCP).

We also implemented LRU cache to cache recently visited websites. 

#### Instruction on Running

The project structure is like the following:


​	|— docker-compose.yml

​	|— README.md

​	|— testcases.txt

​	|— src

​		|-	|— Dockerfile

​		|-	|— Makefile

​		|-	|— Other source code files



Running Instructions

- docker compose-up -d
- Use Firefox (do not forget to set proxy server) to validate, please see in testcases.txt
- cd ./logs
- use cat/less to see the content in proxy.log file



