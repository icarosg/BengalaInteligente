import { SerialPort } from 'serialport';
import { WebSocketServer } from 'ws';

const PORT = 3001;
const wsServer = new WebSocketServer({ port: PORT });

const serial = new SerialPort({ path: '/dev/ttyACM0', baudRate: 115200 });

wsServer.on('listening', () => console.log(`WebSocket rodando em ws://localhost:${PORT}`));

serial.on('open', () => console.log('UART conectada em /dev/ttyACM0'));

serial.on('data', (data) => {
    const message = data.toString().trim();
    console.log('UART:', message);
    wsServer.clients.forEach((client) => {
        if (client.readyState === client.OPEN) client.send(message);
    });
});

// recebe dados do WebSocket e envia para a UART
wsServer.on('connection', (ws) => {
    console.log('Cliente conectado');

    ws.on('message', (message) => {
        console.log('Recebido do WebSocket:', message);
        serial.write(message + '\n', (err) => {
            if (err) console.error('Erro ao enviar para UART:', err.message);
            else console.log('Enviado à UART:', message);
        });
    });

    ws.on('close', () => console.log('Cliente desconectado'));
});
