# Configuração

Primeiro adota a placa no **ESPHome Device Builder** (Home Assistant → ESPHome → *Adopt*). O YAML adotado importa `devices/ufh-s3-eth.yaml@stable`; junta-lhe as linhas abaixo e carrega em **Install**.

## Número de zonas e relés

```yaml
substitutions:
  zone_count: "8"     # 1-14, zona N = relé N
  pump_relay: "9"     # depois da última zona
  heat_relay: "10"
```

## Placa de relés que liga com a entrada em HIGH

```yaml
substitutions:
  relay_inverted: "false"
```

## MCP23017 noutro endereço

```yaml
substitutions:
  mcp23017_address: "0x21"
```

## Endereço IP fixo { #fixed-ip-address }

```yaml
ethernet:
  manual_ip:
    static_ip: 192.168.1.50
    gateway: 192.168.1.1
    subnet: 255.255.255.0
    dns1: 192.168.1.1
```

## Termóstatos na placa { #thermostats-on-the-device }

Opcional: uma entidade de clima por zona que liga o seu relé sozinha, para o aquecimento continuar com o Home Assistant em baixo (a temperatura continua a vir de um sensor do Home Assistant):

```yaml
packages:
  thermostat1: !include
    file: github://skydoy/ESPHome_UFH/ufh/thermostat-zone.yaml@stable
    vars: { zone: 1, room: "Living Room", temperature_entity: sensor.living_room_temperature }
```

Predefinições Comfort 21 °C, Eco 18 °C, Away 12 °C; mínimo de 5 min ligada / 5 min desligada por zona. Exemplo completo: [`examples/my-ufh-with-thermostats.yaml`](https://github.com/skydoy/ESPHome_UFH/blob/main/examples/my-ufh-with-thermostats.yaml).

Sondas DS18B20 locais podem ir para o **GPIO15** (1-Wire, reservado no pacote da placa): junta um barramento `one_wire` e sensores `dallas_temp` e aponta o `sensor:` do termóstato para eles.

## Página web com login / sem página web

```yaml
web_server:
  auth:
    username: admin
    password: !secret web_password
```

Para tirar a página: `web_server: !remove`.
