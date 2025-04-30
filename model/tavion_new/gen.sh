while read line; do
    filename=$line
    name="${line%.*}"
    ppm_name="${name}.ppm"
    vq_name="${name}.vq"
    magick "${filename}" "${ppm_name}"

    #python ~/model_generator/color_convert.py $filename argb1555 twiddled non_mipmapped $data_name
    ./gen/k_means/k_means_vq "${ppm_name}" "${vq_name}" &
    make ${vq_name}.h
done

for job in `jobs -p`
do
    echo wait $job
    wait $job
done
