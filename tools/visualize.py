#!/usr/bin/env python3

import json
import matplotlib.pyplot as plt
import pandas as pd
import numpy as np
from pathlib import Path
from typing import Dict, List, Tuple, Optional
import seaborn as sns
from dataclasses import dataclass
import matplotlib.patches as mpatches

@dataclass
class BenchmarkResult:
    name: str
    implementation: str
    complexity: str
    operation: str
    pattern: str
    cpu_time: float
    real_time: float
    items_per_second: Optional[float]
    iterations: int

def parse_benchmark_name(name: str) -> Tuple[str, str, str, str]:
    parts = name.split('/')
    implementation = parts[0]
    complexity = parts[1] if len(parts) > 1 else "Unknown"
    operation = parts[2] if len(parts) > 2 else "Unknown"
    pattern = parts[3] if len(parts) > 3 else "Single"
    return implementation, complexity, operation, pattern

def load_benchmark_data(json_path: str) -> List[BenchmarkResult]:
    with open(json_path, 'r') as f:
        data = json.load(f)

    results: List[BenchmarkResult] = []
    for benchmark in data['benchmarks']:
        impl, complexity, operation, pattern = parse_benchmark_name(benchmark['name'])
        result = BenchmarkResult(
            name=benchmark['name'],
            implementation=impl,
            complexity=complexity,
            operation=operation,
            pattern=pattern,
            cpu_time=benchmark['cpu_time'],
            real_time=benchmark['real_time'],
            items_per_second=benchmark.get('items_per_second'),
            iterations=benchmark['iterations']
        )
        results.append(result)
    return results

def create_accurate_speedup_heatmap(results: List[BenchmarkResult], output_dir: Path, display: bool) -> None:
    df = pd.DataFrame([{
        'Implementation': r.implementation,
        'Complexity': r.complexity,
        'Operation': r.operation,
        'Pattern': r.pattern,
        'CPU Time (ns)': r.cpu_time
    } for r in results])

    operations = ['Construction', 'Assignment', 'Copy', 'Move']
    complexities = ['Simple', 'Complex', 'Large']

    speedup_matrix = np.zeros((len(operations), len(complexities) * 2))  # *2 for visit and switch
    labels = []

    for i, op in enumerate(operations):
        for j, complexity in enumerate(complexities):
            # Get std::variant baseline
            std_data = df[
                (df['Implementation'] == 'std::variant') &
                (df['Operation'] == op) &
                (df['Complexity'] == complexity)
                ]['CPU Time (ns)']

            if len(std_data) > 0:
                std_time = std_data.iloc[0]

                # vrt::variant visit
                vrt_visit_data = df[
                    (df['Implementation'] == 'vrt::variant') &
                    (df['Operation'] == op) &
                    (df['Complexity'] == complexity) &
                    (df['Pattern'] == 'Single')
                    ]['CPU Time (ns)']

                if len(vrt_visit_data) > 0 and vrt_visit_data.iloc[0] > 0:
                    speedup = std_time / vrt_visit_data.iloc[0]
                    speedup_matrix[i, j*2] = min(speedup, 100)  # Cap at 100x for visualization

                # boost::variant
                boost_data = df[
                    (df['Implementation'] == 'boost::variant') &
                    (df['Operation'] == op) &
                    (df['Complexity'] == complexity)
                    ]['CPU Time (ns)']

                if len(boost_data) > 0 and boost_data.iloc[0] > 0:
                    speedup = std_time / boost_data.iloc[0]
                    speedup_matrix[i, j*2 + 1] = speedup

    # Create labels for columns
    for complexity in complexities:
        labels.extend([f'{complexity}\nvrt', f'{complexity}\nboost'])

    # Create heatmap
    fig, ax = plt.subplots(figsize=(14, 8))

    # Use a custom colormap that emphasizes extreme values
    im = ax.imshow(speedup_matrix, cmap='RdYlBu_r', aspect='auto', vmin=0.5, vmax=20)

    # Set ticks and labels
    ax.set_xticks(range(len(labels)))
    ax.set_xticklabels(labels, fontsize=10)
    ax.set_yticks(range(len(operations)))
    ax.set_yticklabels(operations, fontsize=12)

    # Add text annotations with actual speedup values
    for i in range(len(operations)):
        for j in range(len(labels)):
            value = speedup_matrix[i, j]
            if value > 0:
                if value >= 100:
                    text = '100x+'
                    color = 'white'
                elif value >= 10:
                    text = f'{value:.0f}x'
                    color = 'white'
                else:
                    text = f'{value:.1f}x'
                    color = 'black' if value < 5 else 'white'
                ax.text(j, i, text, ha='center', va='center',
                        color=color, fontweight='bold', fontsize=10)

    # Add colorbar
    cbar = plt.colorbar(im, ax=ax, fraction=0.046, pad=0.04)
    cbar.set_label('Speedup Factor vs std::variant', fontsize=12, fontweight='bold')

    plt.title('Performance Speedup Heatmap\n(Relative to std::variant baseline)',
              fontsize=16, fontweight='bold', pad=20)
    plt.xlabel('Implementation & Complexity', fontsize=12, fontweight='bold')
    plt.ylabel('Operation Type', fontsize=12, fontweight='bold')

    plt.tight_layout()
    plt.savefig(output_dir / 'accurate_speedup_heatmap.png', dpi=300, bbox_inches='tight')
    if display:
        plt.show()
    else:
        plt.close()

def create_separate_operation_charts(results: List[BenchmarkResult], output_dir: Path, display: bool) -> None:
    df = pd.DataFrame([{
        'Implementation': r.implementation,
        'Complexity': r.complexity,
        'Operation': r.operation,
        'Pattern': r.pattern,
        'CPU Time (ns)': r.cpu_time
    } for r in results])

    operations = ['Construction', 'Assignment', 'Copy', 'Move']

    for operation in operations:
        operation_data = df[
            (df['Operation'] == operation) &
            (df['Pattern'] == 'Single')
            ].copy()

        if operation_data.empty:
            continue

        # Handle extreme values
        operation_data['CPU Time (ns)'] = operation_data['CPU Time (ns)'].replace(0, 0.001)

        plt.figure(figsize=(12, 8))

        pivot_data = operation_data.pivot_table(
            values='CPU Time (ns)',
            index='Complexity',
            columns='Implementation',
            aggfunc='mean'
        )

        # Create the plot
        ax = pivot_data.plot(kind='bar', width=0.8, figsize=(12, 8))

        # Set log scale only if the range is large
        max_val = pivot_data.max().max()
        min_val = pivot_data.min().min()
        if max_val / min_val > 100:
            ax.set_yscale('log')

        plt.title(f'{operation} Performance Comparison\n(Lower = Better)',
                  fontsize=16, fontweight='bold', pad=20)
        plt.xlabel('Variant Complexity', fontsize=12, fontweight='bold')
        plt.ylabel('CPU Time (nanoseconds)', fontsize=12, fontweight='bold')

        ax.grid(True, alpha=0.3, which='both')
        ax.tick_params(axis='x', rotation=0)

        # Add value labels with smart formatting
        for container in ax.containers:
            labels = []
            for patch in container:
                height = patch.get_height()
                if height < 0.01:
                    labels.append('<0.01ns')
                elif height < 1:
                    labels.append(f'{height:.3f}ns')
                elif height < 10:
                    labels.append(f'{height:.1f}ns')
                else:
                    labels.append(f'{height:.0f}ns')
            ax.bar_label(container, labels, rotation=90, fontsize=9)

        # Add speedup annotations
        if 'std::variant' in pivot_data.columns:
            for complexity in pivot_data.index:
                std_val = pivot_data.loc[complexity, 'std::variant']
                x_pos = list(pivot_data.index).index(complexity)

                if 'vrt::variant' in pivot_data.columns:
                    vrt_val = pivot_data.loc[complexity, 'vrt::variant']
                    if vrt_val > 0:
                        speedup = std_val / vrt_val
                        if speedup > 10:
                            ax.annotate(f'{speedup:.0f}x faster',
                                        xy=(x_pos, vrt_val),
                                        xytext=(10, 20),
                                        textcoords='offset points',
                                        bbox=dict(boxstyle='round,pad=0.3', facecolor='lightgreen', alpha=0.7),
                                        arrowprops=dict(arrowstyle='->', connectionstyle='arc3,rad=0'),
                                        fontsize=9, fontweight='bold')

        plt.legend(bbox_to_anchor=(1.05, 1), loc='upper left')
        plt.tight_layout()
        plt.savefig(output_dir / f'{operation.lower()}_detailed.png', dpi=300, bbox_inches='tight')
        if display:
            plt.show()
        else:
            plt.close()

def create_extreme_values_chart(results: List[BenchmarkResult], output_dir: Path, display: bool) -> None:
    df = pd.DataFrame([{
        'Implementation': r.implementation,
        'Complexity': r.complexity,
        'Operation': r.operation,
        'Pattern': r.pattern,
        'CPU Time (ns)': r.cpu_time,
        'Name': f"{r.implementation}/{r.complexity}/{r.operation}"
    } for r in results])

    # Find the most extreme improvements
    extreme_cases = []

    for complexity in ['Simple', 'Complex', 'Large']:
        for operation in ['Construction', 'Assignment', 'Copy', 'Move']:
            std_data = df[
                (df['Implementation'] == 'std::variant') &
                (df['Operation'] == operation) &
                (df['Complexity'] == complexity) &
                (df['Pattern'] == 'Single')
                ]

            vrt_data = df[
                (df['Implementation'] == 'vrt::variant') &
                (df['Operation'] == operation) &
                (df['Complexity'] == complexity) &
                (df['Pattern'] == 'Single')
                ]

            if len(std_data) > 0 and len(vrt_data) > 0:
                std_time = std_data.iloc[0]['CPU Time (ns)']
                vrt_time = vrt_data.iloc[0]['CPU Time (ns)']

                if vrt_time > 0 and std_time / vrt_time > 5:  # Only show 5x+ improvements
                    speedup = std_time / vrt_time
                    extreme_cases.append({
                        'Case': f'{complexity} {operation}',
                        'std::variant': std_time,
                        'vrt::variant': max(vrt_time, 0.001),  # Avoid zero for log scale
                        'Speedup': speedup
                    })

    if extreme_cases:
        extreme_df = pd.DataFrame(extreme_cases)
        extreme_df = extreme_df.sort_values('Speedup', ascending=True)

        fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(18, 8))

        # Left plot: Actual times
        y_pos = range(len(extreme_df))
        width = 0.35

        ax1.barh([y - width/2 for y in y_pos], extreme_df['std::variant'],
                 width, label='std::variant', color='#ff7f0e', alpha=0.8)
        ax1.barh([y + width/2 for y in y_pos], extreme_df['vrt::variant'],
                 width, label='vrt::variant', color='#2ca02c', alpha=0.8)

        ax1.set_yticks(y_pos)
        ax1.set_yticklabels(extreme_df['Case'])
        ax1.set_xlabel('CPU Time (nanoseconds)')
        ax1.set_title('Extreme Performance Cases\n(Actual Times)')
        ax1.set_xscale('log')
        ax1.legend()
        ax1.grid(True, alpha=0.3)

        # Right plot: Speedup factors
        bars = ax2.barh(y_pos, extreme_df['Speedup'], color='#d62728', alpha=0.8)
        ax2.set_yticks(y_pos)
        ax2.set_yticklabels(extreme_df['Case'])
        ax2.set_xlabel('Speedup Factor (Higher = Better)')
        ax2.set_title('Speedup Factors\n(vrt::variant vs std::variant)')
        ax2.grid(True, alpha=0.3)

        # Add speedup labels
        for i, (bar, speedup) in enumerate(zip(bars, extreme_df['Speedup'])):
            if speedup >= 100:
                label = '100x+'
            else:
                label = f'{speedup:.1f}x'
            ax2.text(bar.get_width() + speedup * 0.02, bar.get_y() + bar.get_height()/2,
                     label, ha='left', va='center', fontweight='bold')

        plt.suptitle('Most Extreme Performance Improvements', fontsize=16, fontweight='bold')
        plt.tight_layout()
        plt.savefig(output_dir / 'extreme_performance_cases.png', dpi=300, bbox_inches='tight')
        if display:
            plt.show()
        else:
            plt.close()

def create_hero_chart(results: List[BenchmarkResult], output_dir: Path, display: bool) -> None:
    df = pd.DataFrame([{
        'Implementation': r.implementation,
        'Complexity': r.complexity,
        'Operation': r.operation,
        'Pattern': r.pattern,
        'Items/Second': r.items_per_second or 0
    } for r in results])

    visit_data = df[
        (
                ((df['Operation'] == 'Visit') & (df['Pattern'] == 'Batch')) |
                ((df['Operation'] == 'Switch') & (df['Pattern'] == 'Batch')) |
                ((df['Operation'] == 'Visit') & (df['Pattern'] == 'Single'))
        ) &
        (df['Items/Second'] > 0)
        ].copy()

    visit_data['Label'] = visit_data.apply(
        lambda row: 'vrt::variant (switch)' if row['Implementation'] == 'vrt::variant' and row['Operation'] == 'Switch'
        else 'vrt::variant (visit)' if row['Implementation'] == 'vrt::variant' and row['Operation'] == 'Visit'
        else row['Implementation'],
        axis=1
    )

    batch_only = visit_data[
        ((visit_data['Operation'] == 'Visit') & (visit_data['Pattern'] == 'Batch')) |
        ((visit_data['Operation'] == 'Switch') & (visit_data['Pattern'] == 'Batch'))
        ].copy()

    if not batch_only.empty:
        plt.figure(figsize=(16, 10))
        pivot_data = batch_only.pivot_table(
            values='Items/Second',
            index='Complexity',
            columns='Label',
            aggfunc='mean'
        )

        ax = pivot_data.plot(kind='bar', width=0.8, figsize=(16, 10))
        plt.title('Variant Performance Comparison: All Implementations',
                  fontsize=20, fontweight='bold', pad=20)
        plt.xlabel('Variant Complexity', fontsize=14, fontweight='bold')
        plt.ylabel('Operations per Second', fontsize=14, fontweight='bold')

        ax.set_yscale('log')
        ax.grid(True, alpha=0.3, which='both')
        ax.tick_params(axis='x', rotation=0, labelsize=12)
        ax.tick_params(axis='y', labelsize=12)

        colors = ['#1f77b4', '#ff7f0e', '#2ca02c', '#d62728', '#9467bd']
        for i, container in enumerate(ax.containers):
            if i < len(colors):
                for patch in container:
                    patch.set_color(colors[i])

        legend = ax.legend(title='Implementation', bbox_to_anchor=(1.05, 1), loc='upper left', fontsize=11)
        legend.get_title().set_fontsize(12)

        for container in ax.containers:
            labels = []
            for v in container:
                height = v.get_height()
                if height > 0:
                    if height >= 1e9:
                        labels.append(f'{height/1e9:.2f}G')
                    elif height >= 1e6:
                        labels.append(f'{height/1e6:.0f}M')
                    else:
                        labels.append(f'{height:.0f}')
                else:
                    labels.append('')
            ax.bar_label(container, labels, rotation=90, fontsize=9)

        plt.tight_layout()
        plt.savefig(output_dir / 'hero_performance_chart.png', dpi=300, bbox_inches='tight')
        if display:
            plt.show()
        else:
            plt.close()

def print_summary(results: List[BenchmarkResult]) -> None:
    print("Enhanced charts generated successfully!")
    print("Files created:")
    print("- hero_performance_chart.png")
    print("- accurate_speedup_heatmap.png")
    print("- extreme_performance_cases.png")
    print("- construction_detailed.png")
    print("- assignment_detailed.png")
    print("- copy_detailed.png")
    print("- move_detailed.png")

def main() -> None:
    import argparse

    parser = argparse.ArgumentParser(description='Enhanced vrt::variant benchmark visualization')
    parser.add_argument('json_file', help='Path to Google Benchmark JSON output file')
    parser.add_argument('--output-dir', '-o', default='.', help='Output directory')
    parser.add_argument('--no-display', action='store_true', help='Save only, no display')

    args = parser.parse_args()

    if not Path(args.json_file).exists():
        print(f"Error: {args.json_file} not found")
        return

    results = load_benchmark_data(args.json_file)
    output_dir = Path(args.output_dir)
    output_dir.mkdir(exist_ok=True)

    plt.style.use('seaborn-v0_8')

    create_hero_chart(results, output_dir, not args.no_display)
    create_accurate_speedup_heatmap(results, output_dir, not args.no_display)
    create_extreme_values_chart(results, output_dir, not args.no_display)
    create_separate_operation_charts(results, output_dir, not args.no_display)
    print_summary(results)

if __name__ == "__main__":
    main()
