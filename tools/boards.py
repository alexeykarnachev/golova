import json
import math
import struct
from collections import namedtuple
from pathlib import Path

THIS_DIR = Path(__file__).parent
BOARDS_FILE_PATH = THIS_DIR / "boards.json"
RESOURCES_DIR = THIS_DIR / ".." / "resources"
OUT_FILE_PATH = RESOURCES_DIR / "boards"
ITEM_SPRITES_DIR = RESOURCES_DIR / "items" / "sprites"

Board = namedtuple(
    "Board",
    (
        "n_misses_allowed",
        "n_hits_required",
        "correct_items",
        "wrong_items",
        "rule",
    ),
)


def main():
    boards = []
    with open(BOARDS_FILE_PATH) as f:
        boards = [Board(**b) for b in json.load(f)]

    # Assert boards correctness
    for board in boards:
        items = board.correct_items + board.wrong_items
        n_items = len(items)
        n_items_sqrt = math.sqrt(n_items)

        assert board.n_hits_required <= n_items
        assert int(n_items_sqrt) == n_items_sqrt
        for item in items:
            assert " " not in item, item
            assert all(ord(c) < 128 for c in item), item
            assert Path(ITEM_SPRITES_DIR / f"{item}.png").is_file(), item

    # Serialize boards
    with open(OUT_FILE_PATH, "wb") as f:
        for board in boards:
            items = board.correct_items + board.wrong_items

            # n_misses_allowed
            f.write(struct.pack("i", board.n_misses_allowed))
            f.write(b"\x1F")

            # n_hits_required
            f.write(struct.pack("i", board.n_hits_required))
            f.write(b"\x1F")

            # n_correct_items
            f.write(struct.pack("i", len(board.correct_items)))
            f.write(b"\x1F")

            # items
            for item in items:
                f.write(item.encode("utf-8"))
                f.write(b'\x00')
            f.write(b"\x1F")

            # rule
            f.write(board.rule.encode("utf-8"))
            f.write(b'\x00')
            f.write(b"\x1F")
            f.write(b"\x1E")

        f.write(b"\x1C")


if __name__ == "__main__":
    main()
