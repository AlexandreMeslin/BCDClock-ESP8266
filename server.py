'''
Created on 21 de set de 2019

@author: meslin
'''
from sys import argv, stderr
from socket import getaddrinfo, socket
from socket import AF_INET, SOCK_STREAM, AI_ADDRCONFIG, AI_PASSIVE, IPPROTO_TCP, SOL_SOCKET, SO_REUSEADDR
from posix import abort
import _thread

def getEnderecoHost(porta):
    try:
        enderecoHost = getaddrinfo(None, porta, family=AF_INET, type=SOCK_STREAM, proto=IPPROTO_TCP, flags=AI_ADDRCONFIG | AI_PASSIVE)
    except:
        print("Não obtive informações sobre o servidor (???)", file=stderr)
        abort()
    return enderecoHost

def criaSocket(enderecoServidor):
    fd = socket(enderecoServidor[0][0], enderecoServidor[0][1])
    if not fd:
        print("Não consegui criar o socket", file=stderr)
        abort();
    return fd

"""
From: http://www.unixguide.net/network/socketfaq/4.5.shtml
What exactly does SO_REUSEADDR do?
  This socket option tells the kernel that even if this port is busy (in
  the TIME_WAIT state), go ahead and reuse it anyway.  If it is busy,
  but with another state, you will still get an address already in use
  error.  It is useful if your server has been shut down, and then
  restarted right away while sockets are still active on its port.  You
  should be aware that if any unexpected data comes in, it may confuse
  your server, but while this is possible, it is not likely.

  It has been pointed out that "A socket is a 5 tuple (proto, local
  addr, local port, remote addr, remote port).  SO_REUSEADDR just says
  that you can reuse local addresses.  The 5 tuple still must be
  unique!" by Michael Hunter (mphunter@qnx.com).  This is true, and this
  is why it is very unlikely that unexpected data will ever be seen by
  your server.  The danger is that such a 5 tuple is still floating
  around on the net, and while it is bouncing around, a new connection
  from the same client, on the same system, happens to get the same
  remote port.  This is explained by Richard Stevens in ``2.7 Please
  explain the TIME_WAIT state.''.
"""
def setModo(fd):
    # https://stackoverflow.com/questions/5040491/python-socket-doesnt-close-connection-properly
    fd.setsockopt(SOL_SOCKET, SO_REUSEADDR, 1)
    return 

def bindaSocket(fd, porta):
    try:
        fd.bind(('', porta))
    except:
        print("Erro ao dar bind no socket do servidor", porta, file=stderr)
        abort()
    return 

def escuta(fd):
    try:
        fd.listen(1)
    except:
        print("Erro ao começar a escutar a porta", file=stderr)
        abort()
    print("Iniciando o serviço");
    return 

def conecta(fd):
    (con, cliente) = fd.accept()
    print("Servidor conectado com", cliente)
    return con

def fazTudo(fd):
    while True:
        buffer = fd.recv(1024).decode("utf-8")
        if not buffer:
            break
#        print('==>', buffer)
        print(buffer)
        #fd.send(bytearray(buffer, 'utf-8'))
    print("Conexão terminada com", fd)
    fd.close()
    return

def main():
    if len(argv) == 2:
        porta = argv[1]
    else:
        porta = 8752
    enderecoHost = getEnderecoHost(porta)
    fd = criaSocket(enderecoHost)
    setModo(fd)
    bindaSocket(fd, porta)
    print("Servidor pronto em", enderecoHost)
    escuta(fd)
    while True:
        con = conecta(fd)
        if con == -1:
            continue
        _thread.start_new_thread(fazTudo, (con,))
    return 

if __name__ == '__main__':
    main()
    