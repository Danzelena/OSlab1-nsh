import subprocess

def run_nsh(input_str):
    try:
        process = subprocess.Popen(["./nsh"], stdin=subprocess.PIPE, stdout=subprocess.PIPE, text=True)
        
        output, error = process.communicate(input=input_str)
        
        if process.returncode != 0:
            print("Error", error)
            return None
        return output
    except Exception as e:
        print("Error", e)
        return None
    
if __name__ == "__main__":
    input_str = "ls | wc -l\nexit" 
    output = run_nsh(input_str)
    if output:
        print("Output:", output)