def read_numbers_from_file(filename):
    with open(filename, 'r') as file:
        lines = file.readlines()
        numbers = [float(line.strip()) for line in lines]
    return numbers

def calculate_average(numbers):
    if not numbers:
        return 0.0
    return sum(numbers) / len(numbers)

def main():
    filename = "pathoram_execution_times.txt"
    # filename = "trivialoram_execution_times.txt"

    try:
        numbers = read_numbers_from_file(filename)
        average = calculate_average(numbers)
        print("Numbers in file:", numbers)
        print("Average value:", average)
    except FileNotFoundError:
        print(f"Error: File '{filename}' not found.")
    except ValueError:
        print(f"Error: File '{filename}' contains non-numeric data.")

if __name__ == "__main__":
    main()
