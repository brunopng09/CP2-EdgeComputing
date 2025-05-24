
# Monitor de Ambiente com Arduino

Este projeto consiste em um sistema de monitoramento de ambiente utilizando Arduino. Ele realiza medições de **temperatura**, **umidade** e **luminosidade**, exibindo os dados em um display LCD I2C, indicando o status do ambiente (OK ou RUIM) por meio de LEDs e um alarme sonoro. Além disso, armazena registros periódicos dos dados na EEPROM e permite a leitura dos registros via monitor serial.

## Funcionalidades

- Medição de:
  - Temperatura (°C, °F ou Kelvin)
  - Umidade (%)
  - Luminosidade (%)
- Indicação do status do ambiente:
  - **LED verde**: Ambiente OK
  - **LED vermelho e alarme sonoro**: Ambiente RUIM
- Interface LCD com:
  - Exibição de dados em diferentes telas
  - Ícones personalizados
  - Relógio em tempo real (RTC)
- Registro dos dados na EEPROM a cada 30 segundos
- Leitura automática dos registros no monitor serial

## Componentes Utilizados

- Arduino Uno (ou similar)
- Display LCD 16x2 com interface I2C
- Sensor de temperatura e umidade DHT22
- Sensor de luminosidade (LDR + resistor)
- Módulo RTC DS1307
- LEDs (verde e vermelho)
- Buzzer (alarme sonoro)
- Botões para navegação
- EEPROM interna do Arduino
- Resistores diversos

## Bibliotecas Necessárias

- [LiquidCrystal_I2C](https://github.com/johnrickman/LiquidCrystal_I2C) – para controle do display LCD
- [RTClib](https://github.com/adafruit/RTClib) – para o módulo RTC
- [DHT sensor library](https://github.com/adafruit/DHT-sensor-library) – para o sensor DHT22
- Wire – comunicação I2C (nativa do Arduino)
- EEPROM – leitura e escrita na memória EEPROM (nativa do Arduino)

## Esquema de Ligações

| Componente | Pino Arduino |
| ----------- | ------------- |
| DHT22       | D2            |
| Botão Navegar | D3          |
| Botão Temperatura | D4      |
| LED Verde   | D10           |
| LED Vermelho| D12           |
| Buzzer      | D13           |
| LDR         | A0 (com resistor de pull-down) |
| LCD I2C     | SDA (A4), SCL (A5) |
| RTC DS1307  | SDA (A4), SCL (A5) |

## Como Funciona

- Na inicialização, o display exibe uma animação com o texto **"Insight"**.
- A navegação é feita pelo botão **"Navegar"**, alternando entre as telas:
  1. **Status do Ambiente:** Exibe se o ambiente está "OK" ou "RUIM", além da data e hora atual.
  2. **Umidade:** Exibe a umidade relativa do ar.
  3. **Temperatura:** Exibe a temperatura, podendo alternar entre °C, °F e Kelvin usando o botão **"Temperatura"**.
  4. **Luminosidade:** Exibe a porcentagem de luminosidade.
- O sistema aciona o alarme sonoro e o LED vermelho se os seguintes limites forem ultrapassados:
  - Temperatura: acima de 16°C ou abaixo de 10°C
  - Umidade: acima de 80% ou abaixo de 60%
  - Luminosidade: acima de 40%
- Dados são registrados na EEPROM com carimbo de data/hora a cada 30 segundos.
- A cada 30 segundos, um registro é exibido automaticamente no monitor serial.

## Limitações

- Capacidade máxima de 100 registros na EEPROM (cíclico — sobrescreve do início quando cheia).
- A leitura do log no monitor serial acontece automaticamente a cada 30 segundos.

## Ajuste de Fuso Horário

- O fuso horário pode ser ajustado pela constante:

```cpp
#define UTC_OFFSET -3  // Fuso horário UTC-3
```

## Como Usar

1. Instale as bibliotecas necessárias na IDE do Arduino.
2. Conecte todos os componentes conforme a tabela de ligações.
3. Faça o upload do código para o Arduino.
4. Abra o monitor serial em 9600 bps para visualizar os registros da EEPROM.
5. Utilize os botões para navegar entre as telas e alterar a unidade de temperatura.

## Autor

Projeto desenvolvido por [Pedro Almeida RM:564711 | Kelwin Silva RM:566348  | Bruno Costa RM:562159 | Gabriel Inague RM:561985  |Luis Balbino RM:566222].
