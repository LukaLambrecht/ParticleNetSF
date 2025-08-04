import os
import sys
import json
import numpy as np
import pandas as pd
from fnmatch import fnmatch
import matplotlib.pyplot as plt


def parse_impacts_file_name(inputfile):
    '''
    Derive parameters such as year and pt-bin from a file name.
    Note: depends on naming convention, modify as needed.
    Expected input is of the form impacts_particlenet_tt1l_0.6to1.00_2022preEE_pt200to800.json
    '''
    parts = os.path.basename(inputfile).replace('.json','').split('_')
    res = {
      'file': inputfile,
      'sample': parts[2],
      'wp': parts[3],
      'year': parts[4],
      'bin': parts[5]
    }
    return res


def load_impacts_json(inputfile):
    
    with open(inputfile, 'r') as f:
        info = json.load(f)
    params_dict = {param["name"]: param["fit"] for param in info["params"]}
    return params_dict
    

def plot_scalefactors(ydata, fig=None, ax=None,
        primary_xticklabels=None, secondary_xticklabels=None,
        horizontal_shift=None,
        **kwargs):

    # initialize the figure
    if fig is None or ax is None: fig, ax = plt.subplots(figsize=(10,4))

    # make the x-axis
    xax = np.arange(len(ydata)).astype(float)
    if horizontal_shift is None:
        horizontal_shift = 0.

    # format the y-values
    central_values = [el[1] for el in ydata]
    error_up = [el[2] - el[1] for el in ydata]
    error_down = [el[1] - el[0] for el in ydata]

    # make the base plot
    ax.errorbar(xax + horizontal_shift, central_values, yerr = [error_down, error_up], fmt='o', linestyle='none', capsize=5, **kwargs)
    ax.set_xlim((-0.5, len(ydata)))
    ax.set_ylim((-2, 2))
    
    # plot aesthetics
    if primary_xticklabels is not None:
        ax.set_xticks(ticks=xax, labels=primary_xticklabels, rotation=60, ha='right')
    if secondary_xticklabels is not None:
        for xval, label in secondary_xticklabels.items():
            ax.text(xval-0.15, -1.95, label, ha='left', va='bottom')
            ax.axvline(xval-0.2, color='grey', linestyle='--')
    ax.set_ylabel('Scale factor')
    ax.grid()
    fig.tight_layout()
    return fig, ax


if __name__=='__main__':

    # command-line args
    # (maybe extend later)
    impactfiles = sys.argv[1:]

    # other settings
    # (hard-coded)
    pois = ['tp2jms', 'tp2jmr']

    # read all data in a convenient structure
    data = {}
    years = []
    wps = []
    ptbins = []
    for impactfile in impactfiles:
        # extract the year, working point and pt-bin from the file name
        info = parse_impacts_file_name(impactfile)
        year = info['year']
        if year not in years: years.append(year)
        wp = info['wp']
        if wp not in wps: wps.append(wp) 
        ptbin = info['bin']
        if ptbin not in ptbins: ptbins.append(ptbin)
        if year not in data.keys(): data[year] = {}
        if wp not in data[year].keys(): data[year][wp] = {}
        if ptbin not in data[year][wp].keys(): data[year][wp][ptbin] = {}
        # read the post-fit parameters
        params = load_impacts_json(impactfile)
        for poi in pois:
            if poi not in params.keys():
                msg = f'WARNING: parameter {poi} not found in {impactfile}.'
                print(msg)
                continue
            data[year][wp][ptbin][poi] = params[poi]

    # sort pt bins
    ptmins = [float(ptbin.split('to')[0].replace('pt', '')) for ptbin in ptbins]
    ptmaxs = [float(ptbin.split('to')[1].replace('pt', '')) for ptbin in ptbins]
    ptbins_sorted = [(ptbin, ptmin, ptmax) for ptbin, ptmin, ptmax in zip(ptbins, ptmins, ptmaxs)]
    ptbins_sorted = sorted(ptbins_sorted, key=lambda x: (x[2], -x[1]))
    ptbins_sorted = [ptbin[0] for ptbin in ptbins_sorted]
    ptbins = ptbins_sorted

    # sort the years
    years_sorted = sorted([y.replace('pre','xxx').replace('post', 'zzz') for y in years])
    years_sorted = [y.replace('xxx', 'pre').replace('zzz', 'post') for y in years_sorted]
    years = years_sorted

    # make plots
    # one plot per working point and parameter of interest, plotting years and pt-bins
    for poi in pois:
        for wp in wps:
            # get the data
            ptticklabels = []
            yearticklabels = {}
            ydata = []
            for year in years:
                if wp not in data[year].keys(): continue
                yearticklabels[len(ydata)] = year
                for ptbin in ptbins:
                    if ptbin not in data[year][wp].keys(): continue
                    ptlabel = 'pT ' + ptbin.replace('pt', '').replace('to', ' - ')
                    ptticklabels.append(ptlabel)
                    if poi not in data[year][wp][ptbin].keys():
                        msg = f'WARNING: parameter {poi} not found for year {year}, WP {wp}, pt-bin {ptbin}.'
                        print(msg)
                        ydata.append((0,0,0))
                        continue
                    ydata.append(data[year][wp][ptbin][poi])
            # make a plot
            fig, ax = plot_scalefactors(ydata, primary_xticklabels=ptticklabels,
                        secondary_xticklabels=yearticklabels)
            wplabel = wp.replace('to', ' - ')
            ax.text(0.02, 1.02, f'POI: {poi}, WP: {wplabel}', transform=ax.transAxes)
            fig.tight_layout()
            fig.savefig(f'test_{poi}_{wp}.png')

    # make plots
    # one plot per parameter of interest, plotting years and pt-bins, and overlaying working points
    for poi in pois:
        
        # initializations
        fig = None
        ax = None

        # make colors
        cmap = plt.get_cmap('cool')
        cvals = np.linspace(0, 1, num=len(wps), endpoint=True)
        colors = {wp: cmap(cvals[cidx]) for cidx, wp in enumerate(wps)}

        # make horizontal shifts
        hshifts = np.linspace(0, 0.5, num=len(wps), endpoint=True)
        hshifts = {wp: hshifts[idx] for idx, wp in enumerate(wps)}
        
        for wp in wps:
            # get the data
            ptticklabels = []
            yearticklabels = {}
            ydata = []
            for year in years:
                if wp not in data[year].keys(): continue
                yearticklabels[len(ydata)] = year
                for ptbin in ptbins:
                    if ptbin not in data[year][wp].keys(): continue
                    ptlabel = 'pT ' + ptbin.replace('pt', '').replace('to', ' - ')
                    ptticklabels.append(ptlabel)
                    if poi not in data[year][wp][ptbin].keys():
                        msg = f'WARNING: parameter {poi} not found for year {year}, WP {wp}, pt-bin {ptbin}.'
                        print(msg)
                        ydata.append((0,0,0))
                        continue
                    ydata.append(data[year][wp][ptbin][poi])
            # make a plot
            wplabel = 'WP: ' + wp.replace('to', ' - ')
            fig, ax = plot_scalefactors(ydata,
                        fig=fig, ax=ax,
                        label=wplabel, color=colors[wp],
                        horizontal_shift=hshifts[wp],
                        primary_xticklabels=ptticklabels,
                        secondary_xticklabels=yearticklabels)
        ax.legend(ncols=len(wps))
        ax.text(0.02, 1.02, f'POI: {poi}', transform=ax.transAxes)
        fig.tight_layout()
        fig.savefig(f'test_{poi}.png')
