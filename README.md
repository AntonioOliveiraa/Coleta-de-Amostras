# ESP32 Sensor Data Logger

Este projeto permite a leitura e armazenamento de dados de temperatura e umidade a partir dos sensores **DS18B20** e **DHT22** utilizando o microcontrolador **ESP32**. Os dados são armazenados em um arquivo **CSV** no sistema de arquivos **SPIFFS** do ESP32 e podem ser acessados ou removidos via um servidor web. Além disso, o dispositivo pode ser controlado via um botão físico, que ativa ou desativa a coleta de dados e o servidor web.

## Funcionalidades

- Leitura de temperatura do sensor **DS18B20**.
- Leitura de temperatura e umidade do sensor **DHT22** com tentativas de leitura caso falhe.
- Armazenamento de dados em um arquivo CSV em **SPIFFS**, com controle para apagar o arquivo ao atingir 1000 amostras.
- Controle de leitura e inicialização de servidor web através de um botão físico.
- Servidor web para baixar o arquivo CSV ou removê-lo do sistema.
- Controle de início/parada da coleta de dados via rotas HTTP.
- Indicação de status de operação com um LED integrado.

## Componentes Usados

- **ESP32**
- **DS18B20** (sensor de temperatura)
- **DHT22** (sensor de temperatura e umidade)
- **Botão push** (para controle do servidor e coleta de dados)
- **LED integrado** (indicador de status)

## Diagrama de Conexões

| Componente  | Pino ESP32 |
|-------------|------------|
| DS18B20     | GPIO 4     |
| DHT22       | GPIO 5     |
| Botão Push  | GPIO 19    |
| LED         | GPIO 2     |

## Como Funciona

### 1. Leitura dos Sensores

Os sensores **DS18B20** e **DHT22** são lidos em intervalos configuráveis (no caso deste projeto, 5 segundos). Os dados lidos incluem:

- **DS18B20**: Temperatura (°C)
- **DHT22**: Temperatura (°C) e Umidade (%)

Caso a leitura do sensor **DHT22** falhe, o sistema tenta ler os dados até três vezes, com um intervalo de 1 segundo entre as tentativas. Se as três tentativas falharem, o programa exibe uma mensagem de erro no console.

### 2. Armazenamento de Dados

Os dados coletados são salvos em um arquivo CSV chamado `data.csv` na memória **SPIFFS**. Cada linha do arquivo contém:

- O número da amostra
- Temperatura do **DS18B20**
- Temperatura do **DHT22**
- Umidade do **DHT22**

Se o número de amostras ultrapassar 1000, o arquivo CSV é apagado e um novo arquivo é criado, resetando a contagem.

### 3. Controle via Botão Push

O sistema inclui um botão push configurado no pino **GPIO 19** que, quando pressionado, alterna entre dois estados:

- **Estado 1 (WiFi Desligado)**: O LED está apagado, a coleta de dados está ativada e o servidor web está desligado.
- **Estado 2 (WiFi Ligado)**: O LED está aceso, a coleta de dados é pausada e o servidor web é iniciado para permitir o download ou remoção do arquivo CSV.

Há um controle de debounce no botão para evitar múltiplos acionamentos acidentais.

### 4. Servidor Web

O servidor web permite as seguintes operações através de rotas HTTP:

- **/download**: Baixa o arquivo `data.csv` contendo os dados de temperatura e umidade.
- **/remove**: Remove o arquivo `data.csv` e reseta a contagem de amostras.
- **/start**: Retoma a coleta de dados dos sensores.
- **/stop**: Pausa a coleta de dados dos sensores.

O servidor utiliza o modo de ponto de acesso do **ESP32**, criando uma rede WiFi com o SSID **ESP32_AP** e a senha **password_1234**.

### 5. Indicador de Status com LED

Um LED integrado ao pino **GPIO 2** indica o estado atual do sistema:

- **LED Apagado**: O WiFi e o servidor estão desligados e a coleta de dados está ativada.
- **LED Aceso**: O WiFi e o servidor web estão ativados e a coleta de dados está pausada.

## Configuração do Ambiente de Desenvolvimento

1. Instale o [Arduino IDE](https://www.arduino.cc/en/Main/Software) (ou outra IDE de sua preferência).
2. Configure a placa ESP32 no Arduino IDE. Vá em **Arquivo > Preferências** e adicione a seguinte URL no campo de **URLs Adicionais para Gerenciadores de Placas**:
   ```
   https://dl.espressif.com/dl/package_esp32_index.json
   ```
3. Vá em **Ferramentas > Placas > Gerenciador de Placas**, procure por "ESP32" e instale a biblioteca.
4. Instale as seguintes bibliotecas necessárias via o Gerenciador de Bibliotecas:
   - **WiFi.h**
   - **OneWire.h**
   - **DallasTemperature.h**
   - **DHT.h**
   - **SPIFFS.h**
   - **ESPAsyncWebServer.h**
   - **ESPmDNS.h**

## Instalação do Projeto

1. Clone o repositório ou baixe os arquivos.
    ```bash	
    git clone https://github.com/AntonioOliveiraa/Coleta-de-Amostras.git
    cd Coleta-de-Amostras
    ```
    Or
    ```bash
    gh repo clone AntonioOliveiraa/Coleta-de-Amostras
    ```
2. Abra o código no Arduino IDE.
3. Carregue o código na sua placa ESP32.
4. Conecte-se à rede **ESP32_AP** com a senha **password_1234** e acesse o IP do ESP32 (geralmente **192.168.4.1**) no navegador.

## Uso do Projeto

1. **Coleta de Dados**: Quando o dispositivo está coletando dados, o LED está apagado. A cada 5 segundos, uma nova amostra é lida e salva no arquivo CSV.
2. **Ativando o Servidor Web**: Pressione o botão para ativar o WiFi e iniciar o servidor web. O LED acenderá, indicando que a coleta de dados foi pausada e o servidor está ativo.
3. **Baixando o Arquivo CSV**: Acesse `http://192.168.4.1/download` no navegador para baixar o arquivo CSV.
4. **Removendo o Arquivo CSV**: Acesse `http://192.168.4.1/remove` para apagar o arquivo CSV.
5. **Parando a Coleta de Dados**: Acesse `http://192.168.4.1/stop` para parar a coleta de dados.
6. **Iniciando a Coleta de Dados**: Acesse `http://192.168.4.1/start` para reiniciar a coleta de dados.

## Notas Finais

- O sistema inicia fazendo a coleta de amostras dos sensores automaticamente por padrão.
- O sistema é reiniciado automaticamente ao atingir 1000 amostras, garantindo que o arquivo CSV não cresça indefinidamente.
- O botão push deve ser usado para alternar entre os estados de coleta de dados e servidor web. Quando o servidor web está ativado, a coleta de dados é pausada.
- Certifique-se de que o ESP32 esteja alimentado de forma estável para evitar quedas de conexão durante o uso do servidor web.
