declare -a textures=(
    "textures/common/caulk"
    "textures/e7/e7walldesign01b"
    "textures/e7/e7steptop2"
    "noshader"
    "textures/e7/e7dimfloor"
    "textures/e7/e7brickfloor01"
    "textures/e7/e7bmtrim"
    "textures/e7/e7sbrickfloor"
    "textures/e7/e7brnmetal"
    "textures/common/clip"
    "textures/e7/e7beam02_red"
    "textures/e7/e7swindow"
    "textures/e7/e7bigwall"
    "textures/e7/e7panelwood"
    "textures/e7/e7beam01"
    "textures/gothic_floor/xstepborder5"
    "textures/liquids/lavahell"
    "textures/e7/e7steptop"
    "textures/gothic_trim/metalblackwave01"
    "textures/stone/pjrock1"
    "textures/skies/tim_hell"
    "textures/common/hint"
    "models/mapobjects/timlamp/timlamp"
    "textures/sfx/flame1side"
    "textures/sfx/flame2"
    "models/mapobjects/gratelamp/gratetorch2"
    "models/mapobjects/gratelamp/gratetorch2b"
)

for t in "${textures[@]}"
do
    filename=$(ls "$t".{tga,jpg} 2>/dev/null)
    if [ -z $filename ]; then
       continue;
    fi
    name="${filename%.*}"
    data_name="${name}.data"
    echo $data_name
    python ~/model_generator/color_convert.py $filename rgb565 non_twiddled $data_name
    (cd .. ; make pk/${data_name}.h)
done
