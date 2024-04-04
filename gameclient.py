import socket
import sys
import time
import threading
import tkinter as tk
from tkinter import font

class MancalaGUI(tk.Tk):
    def __init__(
        self,
        sock,
        reference_size: int = 10,
    ):
        super().__init__()

        self.sock = sock
        self.reference_size = reference_size
        self.player1 = "P1"
        self.player2 = "P2"


        self.board = {
            "top": [4, 4, 4, 4, 4, 4],
            "bottom": [4, 4, 4, 4, 4, 4],
            "top_score": 0,
            "bottom_score": 0,
        }

        self.player_1_goal_label_text = "{player}\n{points:0>2}\n"
        self.player_2_goal_label_text = "\n{points:0>2}\n{player}"



        self.title("Mancala")

        self.font_large = font.Font(size=self.reference_size * 2)
        self.font_med = font.Font(size=self.reference_size)
        self.font_small = font.Font(size=self.reference_size // 2)

        self.mancala_board_frame = tk.Frame(
            master=self,
            padx=self.reference_size,
            pady=self.reference_size,
            background="brown",
            relief=tk.SOLID,
        )

        self.mancala_board_frame.grid(column=0, row=2, sticky="nsew")



        self.player_1_goal_label = tk.Label(
            master=self.mancala_board_frame,
            text="P1\n00\n",
            padx=self.reference_size // 2,
            pady=self.reference_size // 2,
            font=self.font_large,
            width=self.reference_size // 4,
            bg="white",
        )
        self.player_2_goal_label = tk.Label(
            master=self.mancala_board_frame,
            text="\n00\nP2",
            padx=self.reference_size // 2,
            pady=self.reference_size // 2,
            font=self.font_large,
            width=self.reference_size // 4,
            bg="white",
        )

        self.mancala_button_list = []
        for row in range(2):
            for column in range(6):

                def choose_move_closure(location: str, tile: int):
                    return lambda: self.choose_move(location=location, tile=tile)

                mancala_button = tk.Button(
                    master=self.mancala_board_frame,
                    text="4",
                    padx=self.reference_size // 2,
                    pady=self.reference_size // 2,
                    font=self.font_large,
                    command=choose_move_closure(
                        location="top" if row == 0 else "bottom",
                        tile=1 + column if row == 0 else 6 - column,
                    ),
                    width=self.reference_size // 8,
                )

                mancala_button.grid(column=1 + column, row=row, sticky="nsew")

                self.mancala_button_list.append(mancala_button)

        self.player_1_goal_label.grid(column=0, row=0, rowspan=2, sticky="nsew")
        self.player_2_goal_label.grid(column=7, row=0, rowspan=2, sticky="nsew")

        data = self.sock.recv(4096)

        message = data.decode()
        mes = message.split(",")
        print(mes)
        self.who = mes[0]
        loc1 = mes[1]
        loc2 = mes[8]
        loc3 = mes[11]
        num1 = mes[2:8]
        num2 = mes[9:11]
        num3 = mes[12:18]

        self.who_move = mes[18]

        intnum1 = [int(x) for x in num1]
        intnum2 = [int(x) for x in num2]
        intnum3 = [int(x) for x in num3]


        self.update_data(loc1, intnum1)

        self.update_data(loc2, intnum2)

        self.update_data(loc3, intnum3)
        if self.who_move == "player1":
            self.player_1_goal_label.config(
                bg="green",
            )
            self.player_2_goal_label.config(
                bg="white",
            )
        if self.who_move == "player2":
            self.player_2_goal_label.config(
                bg="green",
            )
            self.player_1_goal_label.config(
                bg="white",
            )

        threading.Thread(target=self.listen_server).start()


    def sync_backend_front_end(self):

        self.player_1_goal_label.config(
            text=self.player_1_goal_label_text.format(
                player="YOU" if self.who == "p1" else "P1",
                points=self.board["top_score"],
            )
        )
        self.player_2_goal_label.config(
            text=self.player_2_goal_label_text.format(
                player="YOU" if self.who == "p2" else "P2",
                points=self.board["bottom_score"],
            )
        )

        for index, tile in enumerate((self.board["top"])):
            self.mancala_button_list[index].config(text=tile)

        for index, tile in enumerate(self.board["bottom"]):
            self.mancala_button_list[index + 6].config(text=tile)


    def choose_move(self, location: str, tile: int):
        if ((self.who_move == "player1") and (location != "bottom")):
            str_tile = str(tile)
            self.sock.sendall(str_tile.encode())
        if ((self.who_move == "player2") and (location != "top")):
            str_tile = str(tile)
            self.sock.sendall(str_tile.encode())

    def update_data(self, location: str, data):
        if location == "top":
            self.board["top"] = data

        if location == "bottom":
            self.board["bottom"] = data

        if location == "score":
            self.board["top_score"] = data[0]
            self.board["bottom_score"] = data[1]

        self.sync_backend_front_end()

    def listen_server(self):
        while True:
            try:
                data = self.sock.recv(4096)
                if not data:
                    break
                self.new_data(data)
            except Exception as e:
                print("Error:", e)
                break

    def new_data(self,data):
        message = data.decode()
        if ((message == "You win!") or (message == "You lose!") or (message == "It's a tie!")):
            self.results(message)

        mes = message.split(",")
        print(mes)
        self.who = mes[0]
        loc1 = mes[1]
        loc2 = mes[8]
        loc3 = mes[11]
        # print(loc)
        num1 = mes[2:8]
        num2 = mes[9:11]
        num3 = mes[12:18]

        self.who_move = mes[18]

        # print(num)
        intnum1 = [int(x) for x in num1]
        intnum2 = [int(x) for x in num2]
        intnum3 = [int(x) for x in num3]

        self.update_data(loc1, intnum1)

        self.update_data(loc2, intnum2)

        self.update_data(loc3, intnum3)
        if self.who_move == "player1":
            self.player_1_goal_label.config(
                bg="green",
            )
            self.player_2_goal_label.config(
                bg="white",
            )
        if self.who_move == "player2":
            self.player_2_goal_label.config(
                bg="green",
            )
            self.player_1_goal_label.config(
                bg="white",
            )

    def results(self, data):

        # Tworzenie nowego okna
        sub_window = tk.Toplevel(self)
        sub_window.title("Results")

        # Dodawanie tekstu do nowego okna
        label = tk.Label(sub_window, text=data)
        label.pack(padx=20, pady=20)

        # Dodawanie przycisku "Ok" do nowego okna
        ok_button = tk.Button(sub_window, text="Ok", command=close_windows)
        ok_button.pack(pady=10)

    def close_windows(self):
        sub_window.destroy()  # Zamykanie okna podrzędnego
        self.destroy()  # Zamykanie głównego okna


def main():
    server_host = sys.argv[1]
    server_port = int(sys.argv[2])

    # Create a socket object
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:
        # Connect to the server
        sock.connect((server_host, server_port))
        print("Connected to server")

        mancala = MancalaGUI(sock, 14)
        mancala.mainloop()

        sock.close()

if __name__ == "__main__":
    main()

