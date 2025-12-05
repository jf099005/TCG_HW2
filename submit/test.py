import subprocess

def get_response(input_text: str, agent_path:str ):
    # 啟動 exe
    proc = subprocess.Popen(
        [agent_path],
        stdin=subprocess.PIPE,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True
    )

    # 將 input_text 寫入 stdin，並讀取 stdout/stderr
    stdout, stderr = proc.communicate(input_text)

    return stdout, stderr



def main(args):
    agent_1 = args.agent_1
    agent_2 = args.agent_2

    processor = ""

    board = args.initial_board

    agents = [agent_1, agent_2]
    cur = 0

    while True:
        agent = agents[cur]

        nx = get_response(board, agent)
        board = get_response(board, processor)


if __name == "__main__":
    main()