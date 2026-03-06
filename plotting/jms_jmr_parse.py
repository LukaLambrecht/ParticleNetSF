import os, argparse, json

#input tagger and year to extract SFs
#need to add more years and working points

if __name__=='__main__':
    # read arguments
    parser = argparse.ArgumentParser()
    #parser.add_argument("--wp", help = "Working Point: lowtohigh",type=str,default = "0.82to1.00") # for example "0.82to1.00"
    parser.add_argument('-tag', '--tagger', required=True) # for example, w_md, w_nominal, t_nominal
    parser.add_argument('-year', '--year', required=True) # for example, 2016preVFP, 2016postVFP, 2017
    parser.add_argument('-jettype', '--jettype', default = "ak15")

    args = parser.parse_args()

    #define working points for specific year and tagger
    if args.tagger == 'w_md':
        #pTrange = ['pt200to300', 'pt300to400', 'pt400to800', 'pt200to800']
        pTrange = ['pt200to400', 'pt400to500', 'pt500to800', 'pt200to800']
        if args.year == '2016preVFP':
            wp = [0.7, 0.8, 0.923]
        else:
            #wp = [0.7, 0.8, 0.923]
            wp = [0.58, 0.78, 0.93]
        #elif args.year == '2016postVFP':
        #    wp = [0.642, 0.842, 0.907]
        #elif args.year == '2017':
        #    wp = [0.579, 0.810, 0.891]
    elif args.tagger == 'w_nominal':
        pTrange = ['pt200to300', 'pt300to400', 'pt400to800', 'pt300to800', 'pt200to800']
        if args.year == '2016preVFP':
            wp = []
        elif args.year == '2016postVFP':
            wp = []
        elif args.year == '2017':
            wp = []
    elif args.tagger == 't_nominal':
        pTrange = ['pt300to400', 'pt400to480', 'pt480to600', 'pt600to1200', 'pt200to1200']
        if args.year == '2016preVFP':
            wp = []

    tagger=args.tagger
    year=args.year
    jettype=args.jettype

    for i in wp:
        for pt in pTrange:
            if args.tagger == 'w_md':

                #define and open the input file
                filestring = f"impacts_particlenet_tt1l_{i}to1.00_{year}_{pt}.json"
                with open(filestring, 'r') as f:
                    obj=json.load(f)
                
                #define output file name
                outfileStr=f"wp_{i}to1.00_{tagger}_{year}_{jettype}_{pt}_jms_jmr.txt" 
                txtout=open(outfileStr,"w")
                txtout.write(f"{year} {i}to1.00 {tagger} {pt} {jettype}")
                txtout.write("\n")

                params_dict = {param["name"]: param["fit"]
                for param in obj["params"]}
                names = ['tp3','tp2','tp1']
                for name in names:
                    jmsname = name + 'jms'
                    jmrname = name + 'jmr' 
                    # write jms
                    #txtout.write(f"{jmsname}:")
                    #txtout.write("\n SF: ")
                    #txtout.write(f"{params_dict.get(jmsname)[1]}")
                    fit = params_dict.get(jmsname)
                    if fit:
                        txtout.write(f"{jmsname}:")
                        txtout.write("\n SF: ")
                        txtout.write(f"{params_dict.get(jmsname)[1]}")
                        txtout.write("\n Down Uncertainty: ")
                        txtout.write(f"{params_dict.get(jmsname)[0]-params_dict.get(jmsname)[1]}")
                        txtout.write("\n Up Uncertainty: ")
                        txtout.write(f"{params_dict.get(jmsname)[2]-params_dict.get(jmsname)[1]}")
                    else:
                        txtout.write(f"{jmsname}: not found\n\n")
                    #txtout.write("\n Down Uncertainty: ")
                    #txtout.write(f"{params_dict.get(jmsname)[0]-params_dict.get(jmsname)[1]}")
                    #txtout.write("\n Up Uncertainty: ")
                    #txtout.write(f"{params_dict.get(jmsname)[2]-params_dict.get(jmsname)[1]}")
                    txtout.write("\n")
                    txtout.write("\n")
                    # write jmr
                    #txtout.write(f"{jmrname}:")
                    #txtout.write("\n SF: ")
                    #txtout.write(f"{params_dict.get(jmrname)[1]}")
                    fit = params_dict.get(jmrname)
                    if fit:
                        txtout.write(f"{jmrname}:")
                        txtout.write("\n SF: ")
                        txtout.write(f"{params_dict.get(jmrname)[1]}")
                        txtout.write("\n Down Uncertainty: ")
                        txtout.write(f"{params_dict.get(jmrname)[0]-params_dict.get(jmrname)[1]}")
                        txtout.write("\n Up Uncertainty: ")
                        txtout.write(f"{params_dict.get(jmrname)[2]-params_dict.get(jmrname)[1]}")
                    else:
                        txtout.write(f"{jmrname}: not found\n\n")
                    #txtout.write(f"{jmrname}:")
                    #txtout.write("\n SF: ")
                    #txtout.write(f"{1+params_dict.get(jmrname)[1]}")
                    #txtout.write("\n Down Uncertainty: ")
                    #txtout.write(f"{params_dict.get(jmrname)[0]-params_dict.get(jmrname)[1]}")
                    #txtout.write("\n Up Uncertainty: ")
                    #txtout.write(f"{params_dict.get(jmrname)[2]-params_dict.get(jmrname)[1]}")
                    txtout.write("\n")
                    txtout.write("\n")
                txtout.close()
    

            elif args.tagger == 'w_nominal' or args.tagger == 't_nominal':
                filestring = f"impacts_particlenet_tt1l_{tagger[0]}_{i:.3f}to1.00_{year[0:4]}_{pt}.json"
                with open(filestring, 'r') as f:
                    obj=json.load(f)
                outfileStr=f"wp_{i:.3f}to1.00_{tagger}_{year}_{jettype}_{pt}_jms_jmr.txt" 
                txtout=open(outfileStr,"w")
                txtout.write(f"{year} {i:.3f}to1.00 {tagger} {pt} {jettype}")
                txtout.write("\n")
                params_dict = {param["name"]: param["fit"]
                for param in obj["params"]}
                names = ['tp3','tp2','tp1','other']
                for name in names:
                    jmsname = name + 'jms'
                    jmrname = name + 'jmr' 
                    # write jms
                    txtout.write(f"{jmsname}:")
                    txtout.write("\n SF: ")
                    txtout.write(f"{1+params_dict.get(jmsname)[1]}")
                    txtout.write("\n Down Uncertainty: ")
                    txtout.write(f"{params_dict.get(jmsname)[0]-params_dict.get(jmsname)[1]}")
                    txtout.write("\n Up Uncertainty: ")
                    txtout.write(f"{params_dict.get(jmsname)[2]-params_dict.get(jmsname)[1]}")
                    txtout.write("\n")
                    txtout.write("\n")
                    # write jmr
                    txtout.write(f"{jmrname}:")
                    txtout.write("\n SF: ")
                    txtout.write(f"{1+params_dict.get(jmrname)[1]}")
                    txtout.write("\n Down Uncertainty: ")
                    txtout.write(f"{params_dict.get(jmrname)[0]-params_dict.get(jmrname)[1]}")
                    txtout.write("\n Up Uncertainty: ")
                    txtout.write(f"{params_dict.get(jmrname)[2]-params_dict.get(jmrname)[1]}")
                    txtout.write("\n")
                    txtout.write("\n")
                txtout.close()

