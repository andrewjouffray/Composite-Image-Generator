import os


lines = []

# get the env variables 
for k, v in os.environ.items():



    # add the workdir path to them
    if k.endswith("path"):
        v = os.path.join("/workdir", v)

    # ignores other env variables
    if k[0].islower():
        print(k, "=", v)

        # append them into a list
        line = (k+"="+str(v))
        lines.append(line+"\n")


# write list to the config file
f = open("/code/build/dataset_config.yeet", "w")
f.writelines(lines)
f.close()

