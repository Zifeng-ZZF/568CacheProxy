# erss-hw2-zz204-yh254

The project is designed for ECE 568 homework 2. We built a proxy server which supports HTTP/HTTPS request from browser. It receives and sends request to the destination server then fetches response and returns to the browser. The communication is based on Socket (TCP).

We also implemented LRU cache to cache recently visited websites. 

#### Instruction on Running

The project structure is like the following:

erss-hwk2-zz204-yh254 (project root)

​	|— docker-compose.yml

​	|— README.md

​	|— danger log.txt

​	|— testcases.txt

​	|— src

​		|-	|— Dockerfile

​		|-	|— Makefile

​		|-	|— Other source code files



Running Instructions

- git clone git@gitlab.oit.duke.edu:zz204/erss-hwk2-zz204-yh254.git
- cd erss-hwk2-zz204-yh254/src
- make
- cd ..
- docker compose-up -d
- Use Firefox (do not forget to set proxy server) to validate, please see in testcases.txt
- cd ./logs
- use cat/less to see the content in proxy.log file



