#!/bin/bash
# O -j$(nproc) usa todos os núcleos do processador para compilar mais rápido
# O && garante que só roda se compilar com sucesso
make -j$(nproc) run