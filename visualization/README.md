# TSP Solution Visualizer

This Python script visualizes solutions for the Traveling Salesperson Problem (TSP). It takes a JSON file containing solution data and generates plots for each method and instance, saving them as PNG files.

## Prerequisites

- Python 3.x
- The libraries listed in `requirements.txt`:
  - `pandas`
  - `matplotlib`

## Setup

1.  **Create and activate a virtual environment:**

    ```bash
    # Create a virtual environment
    python -m venv .venv

    # Activate the virtual environment
    # On Windows
    .\.venv\Scripts\activate
    # On macOS/Linux
    source .venv/bin/activate
    ```

2.  **Install the required dependencies:**

    ```bash
    pip install -r requirements.txt
    ```

## Usage

Run the script from the `visualization` directory using the command line. You must provide the path to the input JSON file and the directory where the output images will be saved.

```bash
python visualize.py --input <path_to_results.json> --output <output_directory>
```

### Example

To run the script with the example data provided in the parent repository, you would use the following command from the `visualization` directory:

```bash
python visualize.py --input "..\Assignment 2\results.json" --output "output"
```

This will create an `output` directory (if it doesn't exist) inside the `visualization` directory and save the generated PNG plots there. Each file will be named according to the method and instance, like `Greedy_2-Regret_Method_TSPA.png`.
