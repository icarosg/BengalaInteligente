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
    const [notificar, setNotificar] = useState(false);

    useEffect(() => {
        const ws = new WebSocket('ws://localhost:3001');

        ws.onmessage = (event) => {
            console.log('WebSocket recebeu:', event.data);
            try {
                const json = JSON.parse(event.data);
                setTempoDecorrido(json.tempoDecorrido);
                setNotificar(json.notificar);
            } catch (error) {
                console.error('Erro ao analisar JSON:', error);
                setData(event.data);
            }
        };

        ws.onopen = () => console.log('Conectado ao WebSocket');
        ws.onclose = () => console.log('WebSocket desconectado');
        ws.onerror = (error) => console.error('Erro no WebSocket:', error);


        return () => ws.close();
    }, []);

    return (
        <div className="flex items-center justify-center min-h-screen gap-6">
            <Card className="w-96 shadow-xl">
                <CardHeader className="text-center text-xl font-bold">Dados do dispositivo em tempo real</CardHeader>
                <CardContent className="text-center text-2xl text-blue-600">
                    {tempoDecorrido !== null ? `Duração: ${tempoDecorrido} segundos` : data}
                </CardContent>
            </Card>

            {notificar ?
                <Card className="w-96 shadow-xl bg-red-300">
                    <CardHeader className="text-center text-xl font-bold">Notificação enviada pelo dispositivo</CardHeader>
                    <CardContent className="flex justify-center text-2xl text-blue-600">
                        Notificação recebida!
                    </CardContent>
                </Card>
                :
                <Card className="w-96 shadow-xl bg-green-300">
                    <CardHeader className="text-center text-xl font-bold">Notificação enviada pelo dispositivo</CardHeader>
                    <CardContent className="flex justify-center text-2xl text-blue-600">
                        Sem notificação!
                    </CardContent>
                </Card>}
        </div>
    );
}
