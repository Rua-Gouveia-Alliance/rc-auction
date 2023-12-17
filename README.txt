Servidor e Cliente de Auctions

# Grupo 12

Gonçalo Rua, 102604
João Gouveia, 102611

# Descrição

Sistema de leilões cliente/servidor em C.

# Como usar

1. Compilar
    - "make" na pasta raiz do projeto
2. Executar o servidor
    - ./server [-p port] [-v]
3. Ligar um cliente
    - ./client [-n ip/hostname] [-p port]

# Informação adicional

- Se não especificada, a porta default do cliente/servidor é 58012.
- Se não especificado, o ip/hostname default ao qual o cliente conecta é a própria máquina (127.0.0.1).
- O servidor executa por default em modo não verboso. A flag "-v" ativa o modo verboso.
