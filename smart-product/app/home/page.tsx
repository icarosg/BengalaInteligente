'use client';

import { useEffect, useState } from 'react';
import { Card, CardContent, CardHeader } from '@/components/ui/card';

import * as React from 'react';

import {
    Select,
    SelectContent,
    SelectGroup,
    SelectItem,
    SelectLabel,
    SelectTrigger,
    SelectValue,
} from '@/components/ui/select';

export default function Home() {
    const [data, setData] = useState<string>('Aguardando dados...');
    const [tempoDecorrido, setTempoDecorrido] = useState<number | null>(null);
    const [socket, setSocket] = useState<WebSocket | null>(null); // Estado para armazenar o WebSocket

    useEffect(() => {
        const ws = new WebSocket('ws://localhost:3001');

        ws.onmessage = (event) => {
            console.log('WebSocket recebeu:', event.data);
            try {
                const json = JSON.parse(event.data);
                setTempoDecorrido(json.tempoDecorrido);
            } catch (error) {
                console.error('Erro ao analisar JSON:', error);
                setData(event.data);
            }
        };

        ws.onopen = () => console.log('Conectado ao WebSocket');
        ws.onclose = () => console.log('WebSocket desconectado');
        ws.onerror = (error) => console.error('Erro no WebSocket:', error);

        setSocket(ws);

        return () => ws.close();
    }, []);

    // Função para enviar o valor selecionado ao WebSocket
    const handleSelectChange = (value: string) => {
        if (socket && socket.readyState === WebSocket.OPEN) {
            socket.send(value);
            console.log('Enviado ao WebSocket:', value);
        } else {
            console.error('WebSocket não está conectado');
        }
    };

    return (
        <div className="flex items-center justify-center min-h-screen gap-6">
            <Card className="w-96 shadow-xl">
                <CardHeader className="text-center text-xl font-bold">Dados do dispositivo em tempo real</CardHeader>
                <CardContent className="text-center text-2xl text-blue-600">
                    {tempoDecorrido !== null ? `Duração: ${tempoDecorrido}` : data}
                </CardContent>
            </Card>

            <Card className="w-96 shadow-xl">
                <CardHeader className="text-center text-xl font-bold">Selecione uma informação para enviar ao dispositivo</CardHeader>
                <CardContent className="flex justify-center text-2xl text-blue-600">
                    <Select onValueChange={handleSelectChange}>
                        <SelectTrigger className="w-[180px]">
                            <SelectValue placeholder="Selecione" />
                        </SelectTrigger>
                        <SelectContent>
                            <SelectGroup>
                                <SelectLabel>Opções</SelectLabel>
                                <SelectItem value="remedio">Remédio</SelectItem>
                                <SelectItem value="cuidado">Cuidado</SelectItem>
                                <SelectItem value="aviso">Aviso</SelectItem>
                            </SelectGroup>
                        </SelectContent>
                    </Select>
                </CardContent>
            </Card>
        </div>
    );
}
