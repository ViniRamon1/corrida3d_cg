# modeltest

# Configuração de modeltest para rodar no Visual Studio 

Testado no Visual Studio 2022

Todos os arquivos zipados devem ser extraídos.
Copie as pastas glfw34 e glad, ambos extraídos para a raiz C:/

Crie uma pasta Projetos na raiz C:/

glm e modeltest devem ser copiados dentro da pasta C:/Projetos.

Depois disso, abrir o Developer PowerShell for VS 2022 e executar os seguintes comandos:
cd C:/Projestos
cmake -S . -B out/build
cmake --build out/build --config Release


Dentro da pasta C:/Projetos/modeltest, procure out/build/modeltest.sln e clica duas vezes pra o Visual studio abrir a solução
Botao direito no teste importação e definir como projeto de inicialização. Arquivo e salvar tudo, compilar solução e rodar sem depurar
