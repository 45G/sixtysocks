# Sixtysocks
SOCKS 6 proxy and proxifier (under heavy development)

## Stuff that is notably missing

* Hostname resolution
* Anything else other than CONNECT functionality
* Expiration timers for idle connections, sessions etc.

## Creating a certificate DB

certutil -N -d /home/vlad/Work/nssdb/db/

certutil -A -a -n socks -i socks.crt -t "cCu,," -d /home/vlad/Work/nssdb/db/

openssl pkcs12 -export -out socks.pfx -inkey socks.key -in socks.crt -certfile socks.crt
