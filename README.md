# Fusion Monitor

Fusion Monitor é um sistema de monitoramento de hardware moderno, modular e multiplataforma, desenvolvido em C++20, inspirado em ferramentas consagradas como **HWMonitor** e **MSI Afterburner**, mas projetado desde o início para oferecer uma **base única de monitoramento independente do sistema operacional** utilizado pelo usuário.

O projeto foca em arquitetura limpa, longevidade do código e extensibilidade por sistema operacional, permitindo que a mesma aplicação seja utilizada em ambientes Linux, Windows e macOS sem comprometer o domínio ou a qualidade arquitetural.

O projeto nasce Linux-first, mas é projetado desde o início para suportar Windows e macOS, sem acoplamento entre regras de negócio e detalhes específicos de cada plataforma.

## 🎯 Objetivo do Projeto

O objetivo do Fusion Monitor é fornecer uma base sólida para:

- Monitoramento de CPU, GPU, memória e outros recursos
- Separação clara entre regras de negócio e detalhes de sistema operacional
- Evolução para interfaces gráficas (ex: Qt 6)

## 🧠 Princípios Arquiteturais

O projeto segue rigorosamente:

- Clean Architecture
- DDD (Domain-Driven Design)
- SOLID
- Inversão de Dependência
- Single Responsibility
- C++ moderno (C++20)

### Regras fundamentais

- O domínio não conhece o sistema operacional
- Código específico de sistema operacional vive apenas na infraestrutura
- Providers são intercambiáveis (Strategy + Factory)
- Nenhuma dependência direta entre core e infrastructure

## 🚀 Instalação

🚧 Em desenvolvimento

Ainda não há pacotes de instalação ou builds distribuídos.  
Esta seção será preenchida conforme o projeto evoluir.

## 🧭 Status do Projeto

✔ Arquitetura definida  
✔ Convenções estabilizadas  
✔ Organização por sistema operacional estabelecida  

🚧 Providers Linux em desenvolvimento  
🚧 Interface gráfica (futuro)

## 🏢 Organização

Projeto mantido por **Nyfari**.  
Desenvolvimento focado em qualidade arquitetural, clareza e escala a longo prazo.

## 📜 Licença

Copyright © 2025 Nyfari

Este projeto é licenciado sob a **GNU General Public License v3.0 (GPL-3.0)**.

Você pode usar, estudar, modificar e redistribuir este software, desde que:
- O código-fonte permaneça aberto;
- As obras derivadas também sejam licenciadas sob GPL v3;
- Os devidos créditos sejam mantidos ao projeto original e à Nyfari.

Consulte o arquivo `LICENSE` para mais detalhes.

