import argparse
import json
import os
import pandas as pd
import matplotlib.pyplot as plt


def plot_solution(nodes_df, solution_str, title, output_path):
    """
    Plots the solution and saves it to a file.
    """
    ids = list(map(int, solution_str))

    # Rotate solution to start from smallest ID
    min_id_index = ids.index(min(ids))
    ids = ids[min_id_index:] + ids[:min_id_index]

    # Extract coordinates of selected nodes
    sol_nodes = nodes_df[nodes_df['id'].isin(ids)]
    sol_nodes = sol_nodes.set_index('id').loc[ids]

    # Plot all nodes
    plt.figure(figsize=(8, 8))
    plt.scatter(nodes_df['x'], nodes_df['y'], s=nodes_df['cost']/4,
                color='lightgray', alpha=0.5, label="All nodes")

    # Plot the selected nodes
    plt.scatter(sol_nodes['x'], sol_nodes['y'], s=sol_nodes['cost']/4,
                color='blue', label="Selected nodes")

    # Plot path (closed loop)
    x_coords = sol_nodes['x'].tolist() + [sol_nodes['x'].iloc[0]]
    y_coords = sol_nodes['y'].tolist() + [sol_nodes['y'].iloc[0]]
    plt.plot(x_coords, y_coords, color='red', linewidth=2, label="Solution path")

    # Annotate node IDs
    for node_id, row in sol_nodes.iterrows():
        plt.text(row['x'], row['y'], str(node_id), fontsize=9, ha='center',
                 va='center', color='white', weight='bold')

    # The plot title is REMOVED completely as requested.
    # plt.title(f"{title}", fontsize=10)

    plt.legend()
    plt.xlabel("x")
    plt.ylabel("y")

    # Use tight_layout to ensure the plot uses all available space now that the title is gone.
    plt.tight_layout()

    plt.savefig(output_path)
    plt.close()


def main():
    parser = argparse.ArgumentParser(description='Visualize TSP solutions.')
    parser.add_argument('--input', required=True, help='Path to the input JSON file.')
    parser.add_argument('--output', required=True, help='Directory to save output PNG files.')
    args = parser.parse_args()

    # Create output directory if it doesn't exist
    os.makedirs(args.output, exist_ok=True)

    # Load results data
    with open(args.input, 'r') as f:
        results = json.load(f)

    # Load node data
    nodes_tspa = pd.read_csv('../data/TSPA.csv', sep=';', header=None, names=['x', 'y', 'cost'])
    nodes_tspa['id'] = range(0, len(nodes_tspa))
    nodes_tspb = pd.read_csv('../data/TSPB.csv', sep=';', header=None, names=['x', 'y', 'cost'])
    nodes_tspb['id'] = range(0, len(nodes_tspb))

    data_map = {
        "TSPA": nodes_tspa,
        "TSPB": nodes_tspb
    }

    for instance_name, methods in results.items():
        for method_name, data in methods.items():
            solution = data['best_solution']

            file_name = f"{method_name.replace(' ', '_')}_{instance_name}.png"
            output_path = os.path.join(args.output, file_name)

            title = f"Method: {method_name}\nInstance: {instance_name}" 

            plot_solution(data_map[instance_name], solution, title, output_path)
            print(f"Generated plot for {method_name} on {instance_name} at {output_path}")


if __name__ == '__main__':
    main()
