while read line; do
    filename=$(ls "$line".{tga,jpg} 2>/dev/null)
    if [ -z $filename ]; then
       continue;
    fi
    name="${filename%.*}"
    data_name="${name}.data"
    echo $data_name
    python ~/model_generator/color_convert.py $filename rgb565 twiddled $data_name
    (cd .. ; make pk/${data_name}.h)
done < textures.txt
