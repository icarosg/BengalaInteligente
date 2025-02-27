# Projeto BitDogLab - Bengala Inteligente: Segurança e Autonomia para Deficientes
Visuais

## Descrição do Projeto
O BitDogLab - Bengala Inteligente é uma solução inovadora desenvolvida para proporcionar mais segurança e autonomia a pessoas com deficiência visual. O dispositivo utiliza sensores ultrassônicos para detectar obstáculos e alerta o usuário por meio de sinais sonoros e vibratórios. Além disso, conta com um módulo de comunicação via UART, que permite o envio de alertas em situações de risco, aumentando a proteção do usuário. O sistema foi projetado para ser leve, eficiente e intuitivo, garantindo praticidade no uso diário e contribuindo para uma mobilidade mais segura e independente.

## Como Clonar

1. **Clone o repositório:**
   ```bash
   git clone https://github.com/icarosg/BengalaInteligente.git
   cd BengalaInteligente
   ```
## Como Usar

O projeto está organizado em duas pastas principais:

- **code/**: Contém o código do firmware para a placa embarcada.
- **smart-product/**: Contém a aplicação web para visualização dos dados.

### **Execução do Firmware**
1. Conecte a placa ao computador via USB.
2. Compile e faça o upload do firmware para a placa utilizando a IDE apropriada.

### **Execução da Aplicação Web**
1. Na raiz da pasta BengalaInteligente, acesse a pasta da aplicação:
   ```bash
   cd smart-product
   
2. Inicie o servidor   
Lembrando que a depender do sistema operacional que esteja acessando, é necessário mudar o path no arquivo server.js, para o destino o qual o usb da placa esteja ligada.
Atualmente está no caminho /dev/ttyACM0. Mude para a rota correta, a depender do seu caso.
   ```bash
   node server.js
   
4. Execute a interface web:
   ```bash
   npm run dev

5. Acesse a interface no navegador utilizando o endereço localhost, conforme o parâmetro definido no servidor.

## **Demonstração**

Assista ao vídeo demonstrativo no seguinte link: [Vídeo da solução](https://youtu.be/6nCCD3vMdiw)
