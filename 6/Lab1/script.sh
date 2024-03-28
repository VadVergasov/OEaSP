#!/bin/bash

calculate_hash() {
    local algorithm="$1"
    local file="$2"

    case "$algorithm" in
        md5)
            md5sum "$file" ;;
        sha1)
            sha1sum "$file" ;;
        sha256)
            shasum "$file" -a 256 ;;
        sha512)
            shasum "$file" -a 512 ;;
        *)
            echo "Неподдерживаемый алгоритм хеширования: $algorithm"
            exit 1 ;;
    esac
}

echo "Доступные алгоритмы хеширования: md5, sha1, sha256, sha512"

read -p "Выберите алгоритм хеширования: " algorithm

if [[ "$algorithm" != "md5" && "$algorithm" != "sha1" && "$algorithm" != "sha256" && "$algorithm" != "sha512" ]]; then
    echo "Неподдерживаемый алгоритм хеширования: $algorithm"
    exit 1
fi

read -p "Введите путь к файлу: " file

if [[ ! -f "$file" ]]; then
    echo "Файл не найден: $file"
    exit 1
fi

echo "Хеш-сумма файла:"
calculate_hash "$algorithm" "$file"
