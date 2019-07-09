# Sixtysocks
SOCKS 6 proxy and proxifier (under heavy development)

## Building Sixtysocks

You will need the following packages:
 * qt4-qmake
 * Boost (libboost-dev(el), libboost_system-dev(el), liboost_filesystem-dev(el) liboost_thread-dev(el))
 * Mozilla NSS (libnss-dev(el))
 * Threading Building Blocks (libtbb-dev(el))
 * libsocks6msg ([https://github.com/45G/libsocks6msg])
 * libsocks6util ([https://github.com/45G/libsocks6util])

Then run:

```
qmake
make
```

## Quick start guide

This section is meant to help you quickly setup a transparent SOCKSv6 proxifier and a proxy.

### Creating a certificate DB

If you don't want to run SOCKS on top of TLS, you can skip this section.

Start off by creating a self-signed certificate (you must provide a non-empty CN):

```
openssl req -x509 -newkey rsa:4096 -keyout socks.pem -out socks.pem -days 365
```

Next, create the database:

```
certutil -N -d /path/to/database
```

Add the certificate:

```
certutil -A -a -n socks -i socks.crt -t "cCu,," -d /path/to/database
```

Finally, convert the key to PKCS12 format and add it to the DB:

```
openssl pkcs12 -export -out socks.pfx -inkey socks.key -in socks.crt -certfile socks.crt
pk12util -i socks.pfx -d /path/to/database
```

### Setting up proxification rules

You'll need to get iptables to redirect the traffic that must be proxified to the proxifier.
In this example, all TCP traffic created by the user proxyme will be redirected to the local port 12345.

```
iptables -t nat -N SIXTYSOCKS
iptables -t mangle -N SIXTYSOCKS
iptables -t mangle -N SIXTYSOCKS_MARK

iptables -t nat -A SIXTYSOCKS -p tcp -m owner --uid-owner proxyme -j REDIRECT --to-ports 12345

iptables -t nat -A OUTPUT -p tcp -j SIXTYSOCKS
iptables -t mangle -A PREROUTING -j SIXTYSOCKS
iptables -t mangle -A OUTPUT -j SIXTYSOCKS_MARK
```

### The proxifier

Run the proxifier and proxy as follows:

```
./sixtysocks -m proxify -l 12345 -s <proxy IP> -p <proxy port> -C /path/to/database -S <proxy CN>
```

```
./sixtysocks -m proxy -t <proxy port> -C /path/to/database -n socks
```

If you don't need TLS, use these commands instead:

```
./sixtysocks -m proxify -l 12345 -s <proxy IP> -p <proxy port>
```

```
./sixtysocks -m proxy -l <proxy port>
```

Optionally, you can also require authentication by supplying both the proxifier and proxy with a username and a password.
Just append the following arguments:

```
-U username -P password
```


## Stuff that is notably missing

* Hostname resolution
* Anything else other than CONNECT functionality
* Expiration timers for sessions
