# Silent Hill 2 Discord Presence

Um mod para levar a **atividade do Silent Hill 2 (2024) da Steam para o Discord**. Ele mostra onde você está no jogo, imagens das regiões e o tempo da sua sessão.

É só instalar e abrir o jogo normalmente. Sem outro programa para iniciar e sem janela sobrando na barra de tarefas.

[Read in English](README.md)

## O que aparece no perfil

- A localização ou atividade que o jogo informa à Steam.
- Imagens das regiões, com a capa como alternativa e opção de configurar outras imagens.
- O tempo da sessão, sem reiniciar a contagem quando você muda de área.

O mod lê a atividade local a cada dois segundos e usa o status e os identificadores de região para acompanhar sua localização. Diferenças de outro mundo dependem dos dados enviados pelo jogo; o mod não tenta adivinhar pelo progresso.

## Como instalar

Você precisa do **Silent Hill 2 (2024) na Steam** e do **Discord para desktop**.

1. Feche o jogo e baixe `SH2DiscordPresence-0.1.6-win64.zip` na seção **Releases** deste repositório. Escolha o ZIP do mod, não o download automático "Source code" do GitHub.
2. Na Steam, abra **Silent Hill 2 → Propriedades → Arquivos instalados → Explorar**.
3. Entre em `SHProto/Binaries/Win64`.
4. Extraia o ZIP ali, ao lado de `SHProto-Win64-Shipping.exe`. Mantenha também os arquivos de licença que acompanham o pacote.
5. Abra o Discord e inicie o jogo pela Steam.

**Já usa outros mods?** O pacote inclui o Ultimate ASI Loader. Se você já tem um carregador compatível, aproveite o existente em vez de substituir DLLs ou configurações de outro mod. Coloque o `.asi` e o `.json` numa pasta que ele carregue. Os [detalhes do carregador](docs/DEVELOPMENT.md#loader-notes), em inglês, explicam a configuração usada aqui.

## Vale saber

- **A localização pode demorar um pouco para mudar.** Depois de carregar um save ou entrar em uma área, o próprio jogo pode levar um tempo para atualizar a atividade na Steam.
- **O idioma do RPC é automático.** Se o jogo estiver em português, a presença fica em português. Em inglês ou qualquer outro idioma, o mod traduz a atividade para inglês. A tradução funciona localmente, usando o status e os identificadores de região do jogo, sem serviço externo nem chave de API.
- **O contador é desta sessão.** Menus e pausas entram na contagem. Não é o tempo do save nem o total de horas da Steam.
- **Seu perfil da Steam não precisa ser público.** A leitura é local e não exige senha, chave de API ou token do Discord.
- **O pacote inclui imagens de regiões.** São treze imagens só dos cenários, cobrindo as regiões listadas e a estrada inicial: três capturas e dez artes conceituais oficiais, sem personagens ou inimigos visíveis. As regiões normais, Anoitecer e as versões do Outro Mundo têm imagens distintas. Anoitecer é representado por um interior da Sociedade Histórica. Não são as miniaturas originais do menu de saves. Menus e locais desconhecidos continuam com a capa. A [coleção de imagens e fontes](assets/regions/README.md), em inglês, detalha a cobertura e a hospedagem.

O mod não altera saves nem executáveis do jogo e encerra junto com ele. Não precisa de UE4SS nem de um programa separado rodando em segundo plano.

## Regiões

O [catálogo de regiões](docs/REGIONS.md), em inglês, lista 12 regiões, com Anoitecer e os locais do Outro Mundo separados. O catálogo tem testes automatizados, mas nem todas essas áreas foram visitadas no jogo. A detecção continua dependendo das informações de região enviadas à Steam.

## Configurações

O arquivo que acompanha o mod já vem pronto para usar. Se quiser mudar algo, edite `SH2DiscordPresence.json`, ao lado do `.asi`. As alterações são lidas automaticamente.

O [guia de configuração](docs/CONFIGURATION.md), em inglês, mostra como trocar imagens, ajustar o intervalo e usar outro aplicativo do Discord.

## Deu algum problema?

**Só aparece o texto genérico "Jogando" ou "Playing"?** Carregue seu save e aguarde a atualização da atividade. Se continuar igual, tente mudar de área.

**Não aparece nada no Discord?** Confira se o aplicativo para desktop está aberto e se o compartilhamento de atividade está ativado.

**O jogo está em inglês, mas o RPC continua em português?** O texto da Steam pode usar um idioma diferente do jogo. O mod lê primeiro o idioma salvo nas opções do próprio jogo, então uma Steam em português não força o RPC a ficar em português. Depois de aplicar uma troca de idioma, espere o jogo salvar a opção e atualizar a atividade.

Se continuar, veja `SH2DiscordPresence.log` na pasta do plugin. Uma linha com `Discord confirmed activity` indica que o Discord aceitou aquele texto. Ao relatar um problema, informe a versão do mod, a build do jogo na Steam e as linhas relevantes do log. Revise o que vai compartilhar para não incluir informações pessoais.

## Como remover

Feche o jogo e apague `SH2DiscordPresence.asi`, `SH2DiscordPresence.json` e os logs do mod. Só remova `winmm.dll` e `winmm.ini` se vieram deste pacote e nenhum outro mod depende deles.

## Para quem quer compilar

Com as ferramentas C++ do Windows, CMake e Python instalados:

```powershell
./scripts/build.ps1
./scripts/package.ps1
```

Os requisitos completos, testes e detalhes internos estão nas [notas de desenvolvimento](docs/DEVELOPMENT.md), em inglês.

## Sobre esta versão

A **v0.1.6 é uma atualização beta**. Ela adiciona as imagens que faltavam em Western South Vale, Anoitecer, Hospital do Outro Mundo, Eastern South Vale: Otherworld e Lakeview Hotel: Otherworld. Cada versão usa sua própria imagem em português e inglês. As correções anteriores dos nomes continuam incluídas. Atualize também o JSON para usar as novas imagens. A seleção tem testes automatizados; as novas imagens ainda não foram conferidas em uma sessão do jogo. Os testes anteriores no menu, em Blue Creek e na Zona Oeste usaram a build Steam **17080069**. Outras regiões, transições e combinações com mods ainda precisam de mais testes.

## Créditos

Criado por **Lynn**, sob a [licença MIT](LICENSE).

O projeto usa [Ultimate ASI Loader](https://github.com/ThirteenAG/Ultimate-ASI-Loader) e [nlohmann/json](https://github.com/nlohmann/json). As licenças e os demais avisos estão em [componentes de terceiros](THIRD_PARTY_NOTICES.md).

Este é um projeto de fã, sem vínculo oficial com Konami, Bloober Team, Valve ou Discord. Os nomes e as imagens do jogo pertencem aos respectivos donos. A licença MIT vale para o código, não para as imagens do jogo. As imagens de regiões são carregadas dos arquivos públicos do projeto no GitHub, e a capa usada como alternativa vem da Steam.
