var http = require('http')
var server

function onRequest(req, res) {
    console.log('[' + this.name + ']', req.method, req.url)
    res.writeHead(200, {'Content-Type': 'text/plain'})
    res.end('Hello World\n')
}

function onListening() {
    console.log('[' + this.name + '] Listening at http://' + this.address().address + ':' + this.address().port + '/')
}

ipv4server = http.createServer()
ipv6server = http.createServer()

ipv4server.name = 'ipv4server'
ipv6server.name = 'ipv6server'

ipv4server.on('request', onRequest)
ipv6server.on('request', onRequest)

ipv4server.on('listening', onListening)
ipv6server.on('listening', onListening)

//ipv4server.listen(80, '127.0.0.1')
ipv6server.listen(80, '2001:41d0:800:1e44::2')