# Author: Ancuta Catalin-Mihai
## Environment specifications
The code was implemented on an Arch Linux distribution. As the compiler of choice, **clang** was utilized.
### The code was compiled and run on the following versions:
- Arch Version: **6.8.7-arch1-1**
- Clang Version: **17.0.6**
- Target: **x86_64**
### To compile the code, run the **build.sh** script:
```bash 
./build.sh
```
> [!NOTE]
> If there are any warnings at compilation, ignore them. It will not affect the execution of the program.
The **network** executable will be generated.
### To run the neural network training process invoke the executable:
```bash 
./network
```

### Tests were run on the following datasets
- [CIC_DDoS_2019](https://www.unb.ca/cic/datasets/ddos-2019.html) dataset.
- [KDDCup-99](https://github.com/PacktWorkshops/The-Data-Science-Workshop/blob/master/Chapter09/Dataset/KDDCup99.csv) dataset.
> [!IMPORTANT]
> The dataset utilized are in .csv format. If you want to use other formats than a custom parser must be wrote. You can implement one in the **utils/parser/** directory.

> [!NOTE]
> Other datasets in the .csv format can utilize the **parseCsvData** function, situated in the **utils/parser/** directory. Keep in mind that you must modify the implementation depending on the number of features that you want to have.

### Utilizing other datasets
If you want to utilize other datasets to train an the network, than you must add the dataset path in the **main()** function in the **network.c** file, under the **source** directory.
The following lines need to be modified for the training set sources:
```c
 pSChar8 source[11] = {  "/home/catalin/Datasets/CIC_2019/03-11/Portmap.csv",
                            "/home/catalin/Datasets/CIC_2019/03-11/LDAP.csv",
                            "/home/catalin/Datasets/CIC_2019/03-11/MSSQL.csv",
                            "/home/catalin/Datasets/CIC_2019/03-11/NetBIOS.csv",
                            "/home/catalin/Datasets/CIC_2019/03-11/Syn.csv",
                            "/home/catalin/Datasets/CIC_2019/03-11/UDP.csv",
                            "/home/catalin/Datasets/CIC_2019/01-12/DrDoS_DNS.csv",
                            "/home/catalin/Datasets/CIC_2019/01-12/DrDoS_LDAP.csv",
                            "/home/catalin/Datasets/CIC_2019/01-12/DrDoS_MSSQL.csv",
                            "/home/catalin/Datasets/CIC_2019/01-12/DrDoS_NetBIOS.csv",
                            "/home/catalin/Datasets/CIC_2019/01-12/DrDoS_NTP.csv",
                        };
line: 509
```
And the following line for the testing sources:
```c
processData("/home/catalin/Datasets/CIC_2019/test.csv", &validation); | line: 630
```

### Logging & Debugging
The logging functions that you can utilize are the following:
- NNINFO
- NNWARN
- NNDEBUG
- NNERROR
- NNFATAL
The debug logs can only be seen when setting the **NN_DEBUG_LOG** flag to the value **1** in **utils/logger/logger.h** file.
The logging functions are single threaded and adding them in a mulithreaded context will break the functionality.


