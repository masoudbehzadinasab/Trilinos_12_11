#!/bin/env python3
"""analysis.py

Usage:
  analysis.py -i INPUT... [-o OUTPUT] [-a MODE] [-d DISPLAY] [-s STYLE] [-t TOP]
  analysis.py (-h | --help)

Options:
  -h --help                     Show this screen.
  -i FILE --input-files=FILE    Input file
  -o FILE --output-file=FILE    Output file
  -a MODE --analysis=MODE       Mode [default: setup_timers]
  -d DISPLAY --display=DISPLAY  Display mode [default: print]
  -s STYLE --style=STYLE        Plot style [default: stack]
  -t TOP --top=TOP              Number of timers [default: 10]
"""

import glob
import matplotlib.pyplot as plt
from matplotlib.ticker import FormatStrFormatter
from math import ceil
import numpy             as np
import pandas            as pd
import optparse
import re
from   tableau import *
from   docopt      import docopt
import yaml

def construct_dataframe(yaml_data):
    """Construst a pandas DataFrame from the timers section of the provided YAML data"""
    timers = yaml_data['Timer names']
    data = np.ndarray([len(timers), 8])

    ind = 0
    for timer in timers:
        t = yaml_data['Total times'][timer]
        c = yaml_data['Call counts'][timer]
        data[ind,:] = [
            t['MinOverProcs'],       c['MinOverProcs'],
            t['MeanOverProcs'],      c['MeanOverProcs'],
            t['MaxOverProcs'],       c['MaxOverProcs'],
            t['MeanOverCallCounts'], c['MeanOverCallCounts']
        ]
        ind = ind+1

    return pd.DataFrame(data, index=timers,
        columns=['minT', 'minC', 'meanT', 'meanC', 'maxT', 'maxC', 'meanCT', 'meanCC'])

def setup_timers(input_files, mode, top, ax = None):
    """Show all setup level specific timers ordered by size"""
    ## Choose top timers from the first file
    with open(input_files[0]) as data_file:
        yaml_data = yaml.safe_load(data_file)

    timer_data = construct_dataframe(yaml_data)

    timers = timer_data.index

    timers_f = [x for x in timers   if re.search('.*\(level=', x) != None]    # search level specific
    timers_f = [x for x in timers_f if re.search('Solve', x)      == None]    # ignore Solve timers

    # Select subset of the timer data based on the provided timer labels
    dfs = timer_data.loc[timers_f]
    dfs.sort(columns='maxT', ascending=True, inplace=True)
    timers_f = dfs.index

    # Top few
    if top > len(timers_f):
      print("Warning: there are only ", len(timers_f), " timers to plot.")
    top = min(top, len(timers_f))
    timers_f = dfs.index[-top:]
    dfs = dfs.loc[timers_f]

    ## Check the second file (if present)
    diff_mode = False
    if len(input_files) == 2:
        diff_mode = True

        with open(input_files[1]) as data_file:
            yaml_data = yaml.safe_load(data_file)

        timer_data = construct_dataframe(yaml_data)

        dfs1 = timer_data.loc[timers_f]

    height = 0.4

    if mode == 'display':
        colors = tableau20()

        if diff_mode == False:
            ax.barh(np.arange(len(timers_f))-height, width=dfs['maxT'], height=2*height, color=colors[0])
        else:
            ax.barh(np.arange(len(timers_f)), width=dfs['maxT'],  height=height, color=colors[0], label=input_files[0])
            ax.barh(np.arange(len(timers_f))-height, width=dfs1['maxT'], height=height, color=colors[1], label=input_files[1])
            ax.legend(loc='lower right')

        ax.set_xlabel('Time (s)', fontsize=17)
        ax.xaxis.grid('on')

        ax.set_yticks(np.arange(len(timers_f)))
        ax.set_yticklabels(timers_f)
        ax.set_ylim([-0.5, len(timers_f)-0.5])

    else:
        print(dfs['maxT'])

def muelu_strong_scaling(input_files, mode, ax, top, style):
    """Show scalability of setup level specific timers ordered by size"""

    # Three style options:
    #  - stack        : timers are on top of each other
    #  - stack-percent: same as stack, but shows percentage values on sides
    #  - scaling      : scaling of individual timers

    assert(mode == 'display')

    show_the_rest = 0
    if style == 'stack' or style == 'stack-percent':
        show_the_rest = 1

    ## Determine top timers in the first log file
    with open(input_files[0]) as data_file:
        yaml_data = yaml.safe_load(data_file)

    timer_data = construct_dataframe(yaml_data)

    timers_all = timer_data.index

    timers = [x for x in timers_all if re.search('.*\(level=', x) != None]    # search level specific
    timers = [x for x in timers     if re.search('Solve', x)      == None]    # ignore Solve timers

    # Select subset of the timer data based on the provided timer labels
    dfs = timer_data.loc[timers]
    dfs.sort(columns='maxT', ascending=True, inplace=True)
    timers = dfs.index

    # Choose top few
    if top > len(timers):
      print("Warning: there are only ", len(timers), " timers to plot.")
    top = min(top, len(timers))
    timers = dfs.index[-1:-top-1:-1]

    ## Setup plotting arrays
    nx = len(input_files)
    ny = len(timers) + show_the_rest
    x = np.ndarray(nx)
    y = np.ndarray([ny, nx])
    t = np.ndarray(nx)

    k = 0
    for input_file in input_files:
        with open(input_file) as data_file:
            yaml_data = yaml.safe_load(data_file)

        timer_data = construct_dataframe(yaml_data)

        # Calculate total setup time
        dfs = timer_data.loc['MueLu: Hierarchy: Setup (total)']
        t[k] = dfs['maxT']

        dfs = timer_data.loc[timers]

        x[k] = yaml_data['Number of processes']
        if not show_the_rest:
            y[:,k] = dfs['maxT']
        else:
            y[:-1,k] = dfs['maxT']
            y[-1,k]  = t[k] - sum(y[:-1,k])
        k = k+1

    ## Plot the figure
    if mode == 'display':
        colors = tableau20()

        ## x axis
        ax.set_xlim([x[0], x[-1]])
        ax.set_xscale('log')
        ax.set_xticks(x)
        ax.set_xticklabels([str(i) for i in x])
        ax.set_xlabel('Number of cores', fontsize=17)
        ax.xaxis.set_major_formatter(FormatStrFormatter('%.0f'))
        ax.tick_params(axis='x', which='minor', bottom='off')

        if style == 'stack' or style == 'stack-percent':
            ## Plot
            labels = timers
            if show_the_rest:
                labels = np.append(labels, 'Other')

            ax.stackplot(x, y, colors=colors, labels=labels)

            ## y axis
            ax.set_ylabel('Time (seconds)', fontsize=17)

            if style == 'stack':
                ax.yaxis.grid('on')

            elif style == 'stack-percent':
                # We need to set up 2 Y-axis ticks
                ax2 = ax.twinx()

                for i in [0, -1]:
                    # i =  0: left  y-axis
                    # i = -1: right y-axis
                    yticks = np.ndarray(ny)
                    yticks[0] = 0.5*y[0,i]
                    for k in range(1, ny):
                        yticks[k] = yticks[k-1]  + 0.5*y[k-1,i]  + 0.5*y[k,i]

                    total  = sum(y[:,i])
                    labels = np.ndarray(ny)
                    for k in range(ny):
                        labels[k] = (100*y[k,i])  / total

                    if i == 0:
                        ax.set_yticks([int(x) for x in yticks])
                        ax.set_yticklabels([str(int(i)) + '%' for i in labels])
                    else:
                        ax2.set_ylim(ax.get_ylim())
                        ax2.set_yticks([int(x) for x in yticks])
                        ax2.set_yticklabels([str(int(i)) + '%' for i in labels])

            ## Legend
            handles, labels = ax.get_legend_handles_labels()
            ax.legend(handles[::-1], labels[::-1], loc='upper right')

        elif style == 'scaling':
            width = np.ndarray(ny)
            for k in range(ny):
                width[k] = 9 - min(ceil(y[0,0]/y[k,0]), 8)
                #  width[k] = 9 - min(ceil(y[0,-1]/y[k,-1]), 8)
                #  width[k] = 2
            for k in range(ny):
                y[k,:] = y[k,0] / y[k,:]
            y = np.swapaxes(y, 0, 1)

            plt.gca().set_color_cycle(colors) #[colormap(i) for i in np.linspace(0, 0.9, num_plots)])

            for k in range(ny):
                ax.plot(x, y[:,k], linewidth=width[k])

            ax.plot([x[0], x[-1]], [1.0, x[-1]/x[0]], color='k', linestyle='--')

            # y axis
            ax.yaxis.grid('on')
            #  ax.set_ylim([0, x[-1]/x[0]])
            ax.set_ylim([0.5, x[-1]/x[0]])
            ax.set_xlabel('Scaling', fontsize=17)

            ax.legend(timers, loc='upper left')

def solve_per_level(yaml_data, mode, ax = None):
    """Show solve timers per level"""
    timer_data = construct_dataframe(yaml_data)

    t20 = tableau20()

    labels = ['smoothing', 'residual calculation', 'restriction', 'prolongation', 'coarse']
    colors = {}
    for i in range(len(labels)):
      colors[labels[i]] = t20[i]

    levels = [0, 1, 2, 3, 4, 5, 6]
    for level in levels:
        # check if level exists at all
        level_exists = False
        for op in labels:
            timer_name = 'MueLu: Hierarchy: Solve : ' + op + ' (level=' + str(level) + ')'
            try:
                time = timer_data.loc[timer_name]['maxT']
                level_exists = True
            except KeyError:
                continue

        if not level_exists:
            levels = levels[:level]
            break

        if mode != 'display':
            print('Level ', level)

        height = 0.0
        i = -1
        for op in labels:
            i = i+1
            timer_name = 'MueLu: Hierarchy: Solve : ' + op + ' (level=' + str(level) + ')'
            try:
                time = timer_data.loc[timer_name]['maxT']
            except KeyError:
                # If one uses continue here, it messes up legend colors (due to skipping ax.bar call)
                time = 0

            if mode == 'display':
                ax.bar(level-0.4, time, bottom=height, label=op, color=colors[op], edgecolor='black');
                height = height + time
            else:
                print('  %-20s : %.5f' % (op, time))


    if mode == 'display':
        ax.legend(labels);
        ax.set_xticks(levels);
        ax.set_xlabel('Level')
        ax.set_ylabel('Time (s)')

def nonlinear_history_iterations(yaml_data, mode, ax = None):
    """Show number of linear iterations per nonlinear step across the whole simulation"""
    ticks = []

    colors = tableau20()

    offset = 0
    mx     = 0
    i      = 0
    for step in sorted(yaml_data['Steps']):
        c_step = yaml_data['Steps'][step]

        its = []
        for nlstep in sorted(c_step):
            if nlstep == 'nl_its':
                continue
            ticks.append(str(len(its)))
            its.append(c_step[nlstep]['its'])

        if mode == 'display':
            ax.plot(range(offset, offset + len(its)), its, '-o', color=colors[i % 20])

            offset += len(its)
            mx = max(max(its), mx)
        else:
            print(step, ':', its)

        i += 1

    if mode == 'display':
        ax.set_xlim([-1, len(ticks)])
        ax.set_ylim([0, mx+1]);
        ax.set_xticks(range(len(ticks)))
        ax.set_xticklabels(ticks);
        ax.set_xlabel('Nonlinear iterations index')
        ax.set_ylabel('Number of linear iterations')

def nonlinear_history_residual(yaml_data, mode, ax = None):
    """Show the residual histories for all nonliner steps across the whole simulation"""

    colors = tableau20()

    offset = 0
    i = 0
    for step in sorted(yaml_data['Steps']):
        c_step = yaml_data['Steps'][step]

        if mode == 'print':
            print(step)
        for nlstep in sorted(c_step):
            if nlstep == 'nl_its':
                continue
            res_hist = c_step[nlstep]['res_hist']

            if mode == 'display':
                ax.plot(range(offset, offset+len(res_hist)), res_hist, '-v', markersize=2, color=colors[i % 20])
                offset += len(res_hist)
            else:
                print(nlstep, ':', res_hist)

            i += 1

    if mode == 'display':
        ax.set_xlim([0, offset+1])
        ax.set_yscale('log')
        ax.set_xlabel('Linear iterations index (cumulative)')
        ax.set_ylabel('Relative residual')

def nonlinear_history_solve(yaml_data, mode, ax = None):
    """Show solve time per nonlinear step across the whole simulation (setup time is ignored)"""
    ticks = []

    if yaml_data['scheme'] != 'albany' and yaml_data['scheme'] != 'drekar':
      raise RuntimeError('This mode is for Albany/Drekar only!')

    colors = tableau20()

    offset = 0
    mx     = 0
    i      = 0
    for step in sorted(yaml_data['Steps']):
        c_step = yaml_data['Steps'][step]

        solves = []
        for nlstep in sorted(c_step):
            if nlstep == 'nl_its':
                continue
            ticks.append(str(len(solves)))
            solves.append(c_step[nlstep]['solve_time'])

        if mode == 'display':
            ax.plot(range(offset, offset + len(solves)), solves, '-o', color=colors[i % 20])

            offset += len(solves)
            mx = max(max(solves), mx)
        else:
            print(step, ':', solves)

        i += 1

    if mode == 'display':
        ax.set_xlim([-1, len(ticks)])
        ax.set_ylim([0, mx]);
        ax.set_xticks(range(len(ticks)))
        ax.set_xticklabels(ticks);
        ax.set_xlabel('Nonlinear iterations index')
        ax.set_ylabel('Solve time (s)')


def string_split_by_numbers(x):
    r = re.compile('(\d+)')
    l = r.split(x)
    return [int(x) if x.isdigit() else x for x in l]

if __name__ == '__main__':

    ## Process input
    options = docopt(__doc__)

    input_files = options['--input-files']
    output_file = options['--output-file']
    analysis    = options['--analysis']
    display     = options['--display']
    style       = options['--style']
    top         = int(options['--top'])

    if len(input_files) == 1:
        input_files = glob.glob(input_files[0])
        # Impose sorted order (similar to bash "sort -n"
        input_files = sorted(input_files, key=string_split_by_numbers)

    ## Validate input options
    valid_modes = ['setup_timers', 'solve_per_level', 'nonlinear_history_iterations',
                   'nonlinear_history_residual', 'nonlinear_history_solve',
                   'muelu_strong_scaling']
    if not analysis in valid_modes:
        print("Analysis must be one of: ")
        print(valid_modes)
        raise
    valid_displays = ['print', 'display']
    if not display in valid_displays:
        print("Display must be one of " % valid_displays)
        raise

    valid_styles = ['stack', 'stack-percent', 'scaling']
    if not style in valid_styles:
        print("Style must be one of " % valid_displays)

    ## Setup default plotting
    plt.figure(figsize=(12, 12))
    ax = plt.subplot(111)

    # Remove the plot frame lines.
    ax.spines["top"].set_visible(False)
    ax.spines["bottom"].set_visible(False)
    ax.spines["right"].set_visible(False)
    ax.spines["left"].set_visible(False)

    # Make ticks large enough to read.
    plt.xticks(fontsize = 14)
    plt.yticks(fontsize = 14)

    # Make sure axis ticks are only on the bottom and left. Ticks on the right
    # and top of the plot are generally not needed.
    ax.get_xaxis().tick_bottom()
    ax.get_yaxis().tick_left()

    if analysis == 'muelu_strong_scaling':
        # Scaling studies work with multiple files
        muelu_strong_scaling(input_files, mode=display, ax=ax, style=style, top=top)
    elif analysis == 'setup_timers':
        # Setup timers work with multiple files
        # If there are two files, it compares the timers, using top timers from
        # the first file
        assert(len(input_files) <= 2)
        setup_timers(input_files, mode=display, ax=ax, top=top)
    else:
        # Most analysis studies work with a single file
        # Might as well open it here
        assert(len(input_files) == 1)
        with open(input_files[0]) as data_file:
            yaml_data = yaml.safe_load(data_file)

        if   analysis == 'solve_per_level':
            solve_per_level(yaml_data, mode=display, ax=ax)
        elif analysis == 'nonlinear_history_iterations':
            nonlinear_history_iterations(yaml_data, mode=display, ax=ax)
        elif analysis == 'nonlinear_history_residual':
            nonlinear_history_residual(yaml_data, mode=display, ax=ax)
        elif analysis == 'nonlinear_history_solve':
            nonlinear_history_solve(yaml_data, mode=display, ax=ax)

    ## Save output
    if display != 'print':
        if output_file != None:
            plt.savefig(output_file, bbox_inches='tight')
        else:
            plt.tight_layout()
            plt.show()
