#! /bin/bash

# APARTADO 1
if [ ! -f student-record ] ;
    then echo "El programa student-record no se encuentra en el directorio actual"
         exit
fi

# APARTADO 2
if [ ! -f records.txt ] ; 
    then echo "El archivo records.txt no es un archivo regular o no se encuentra en el directorio"
         exit
fi

# APARTADO 3
records=$(tr '\n' ' ' < records.txt)
echo "$records"

# APARTADO 4
for variables in values
do
    # codigo
done


    
