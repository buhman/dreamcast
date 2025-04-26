while read line; do
    filename=$(ls */"$line".{tga,jpg} 2>/dev/null)
    if [ -z $filename ]; then
       continue;
    fi
    name="${filename%.*}"
    data_name="${name}.data"
    echo $data_name
    python ~/model_generator/color_convert.py $filename argb1555 twiddled non_mipmapped $data_name
    (cd .. ; make bsp/${data_name}.h)
done
