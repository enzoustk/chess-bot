import subprocess
import argparse
import sys
import os

def run_match(engine1_path, engine2_path, games, tc):
    """
    Runs a match between two engines using cutechess-cli.
    """
    # Check if engines exist
    if not os.path.exists(engine1_path):
        print(f"Error: Engine 1 not found at {engine1_path}")
        return
    if not os.path.exists(engine2_path):
        print(f"Error: Engine 2 not found at {engine2_path}")
        return

    # Check if cutechess-cli is available
    cutechess_cmd = "cutechess-cli"
    # Try to find cutechess-cli in path or assume it's installed
    # On WSL/Linux it should be just 'cutechess-cli'
    
    cmd = [
        cutechess_cmd,
        "-engine", f"name=StikerV1", f"cmd={engine1_path}",
        "-engine", f"name=StikerV2", f"cmd={engine2_path}",
        "-each", f"tc={tc}", "proto=uci",
        "-rounds", str(games),
        "-games", "2", # 2 games per round (alternating colors)
        "-repeat", # Repeat opening
        "-pgnout", "match_results.pgn"
    ]

    print(f"Starting match: {engine1_path} vs {engine2_path}")
    print(f"Games: {games * 2}, TC: {tc}")
    print("Command:", " ".join(cmd))

    try:
        process = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, universal_newlines=True)
        
        for line in process.stdout:
            print(line, end='')
            
        process.wait()
        
        if process.returncode != 0:
            print(f"\nMatch finished with error code {process.returncode}")
        else:
            print("\nMatch finished successfully.")
            
    except FileNotFoundError:
        print("Error: cutechess-cli not found. Please install it or ensure it is in your PATH.")
    except Exception as e:
        print(f"An error occurred: {e}")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Run a chess match between two engine versions.")
    parser.add_argument("--engine1", default="./bin/stiker_v1", help="Path to engine 1 executable")
    parser.add_argument("--engine2", default="./bin/stiker_v2", help="Path to engine 2 executable")
    parser.add_argument("--games", type=int, default=10, help="Number of rounds (games = rounds * 2)")
    parser.add_argument("--tc", default="40/60", help="Time control (e.g. 40/60 for 40 moves in 60 sec)")

    args = parser.parse_args()
    
    run_match(args.engine1, args.engine2, args.games, args.tc)
