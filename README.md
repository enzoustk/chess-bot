# [Nome do Projeto]

**Disciplina:** Introdução à Inteligência Artificial  
**Semestre:** 2025.2  
**Professor:** André Fonseca  
# [Nome do Projeto]

**Disciplina:** Introdução à Inteligência Artificial  
**Semestre:** 2025.2  
**Professor:** André Fonseca  
**Turma:** [T03 / T04]

## Integrantes do Grupo
* Enzo Araújo de Souza e Oliveira (20250063249)
* Pedro Lucas Maia de Paiva (20240004960)

## Descrição do Projeto
Este projeto consiste em uma **Engine de Xadrez completa desenvolvida em C++**, capaz de jogar xadrez em alto nível utilizando algoritmos avançados de Inteligência Artificial. O sistema foi projetado para ser modular e eficiente, suportando tanto execução via console quanto uma interface gráfica interativa.

Principais características:
*   **Interface Gráfica (GUI):** Desenvolvida com a biblioteca **SFML**, oferecendo uma experiência visual interativa.
*   **Protocolo UCI:** Suporte ao *Universal Chess Interface*, permitindo integração com plataformas como Lichess (via `lichess-bot`).
*   **Inteligência Artificial:** Implementação robusta utilizando:
    *   Algoritmo **Negamax** com **Poda Alpha-Beta** para busca eficiente.
    *   **Busca de Quiescência** para evitar o efeito horizonte em trocas de peças.
    *   **Tabelas de Transposição (TT)** para memorizar posições já avaliadas.
    *   **Ordenação de Movimentos** com heurísticas de *Killer Moves* e *History Heuristic*.
    *   **Tabelas de Peça-Quadrado (PST)** para avaliação posicional refinada.

## Guia de Instalação e Execução

### Pré-requisitos
*   **Compilador C++17** (ex: g++, clang++)
*   **Biblioteca SFML** (necessária apenas para a interface gráfica)
    *   Ubuntu/Debian: `sudo apt-get install libsfml-dev`
    *   Windows: Baixar do site oficial ou usar gerenciador de pacotes.

### Compilação
O projeto utiliza um `Makefile` para facilitar a compilação. No terminal, execute:

```bash
# Clone o repositório
git clone [https://github.com/enzoustk/chess-bot]

# Entre na pasta do projeto
cd chess-bot

# Instale as dependências
pip install -r requirements.txt
````

### 2. Como Executar

Execute o comando abaixo no terminal para iniciar o servidor local:

```bash
# Exemplo para Streamlit
streamlit run src/app.py
```

Se necessário, especifique a porta ou url de acesso, ex: http://localhost:8501

## Estrutura dos Arquivos

[Descreva brevemente a organização das pastas]

  * `src/`: Código-fonte da aplicação ou scripts de processamento.
  * `notebooks/`: Análises exploratórias, testes e prototipagem.
  * `data/`: Datasets utilizados (se o tamanho permitir o upload).
  * `assets/`: Imagens, logos ou gráficos de resultados.

## Resultados e Demonstração

[Adicione prints da aplicação em execução ou gráficos com os resultados do modelo/agente. Se for uma aplicação Web, coloque um print da interface.]

## Referências

  * [Link para o Dataset original]
  * [Artigo, Documentação ou Tutorial utilizado como base]