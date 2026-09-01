import os
import sys
import json
import numpy as np
import matplotlib.pyplot as plt


def parse_impacts_file_name(inputfile):
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
    # the tagger-score SFs (SF_tp1/tp2/tp3) live in the "POIs" block, not in "params";
    # store their [lo, best, hi] the same way so they can be read like any other parameter
    for poi in info.get("POIs", []):
        params_dict.setdefault(poi["name"], poi["fit"])
    return params_dict


def plot_scalefactors(ydata,poi, fig=None, ax=None,
        primary_xticklabels=None, secondary_xticklabels=None,
        horizontal_shift=0.0, xpositions=None, **kwargs):

    if fig is None or ax is None:
        fig, ax = plt.subplots(figsize=(10, 4))

    if xpositions is None:
        xax = np.arange(len(ydata)).astype(float)
    else:
        xax = np.array(xpositions, dtype=float)
    
    # jms/jmr nuisances are stored in units of the reference variation (5% / 10%),
    # so convert to a physical scale factor via  SF = 1 + scale * value.
    # the tagger-score SFs (SF_tp*) are already absolute -> scale = 1, offset = 0.
    if 'jms' in poi:
        scale, offset = 0.05, 1.0
    elif 'jmr' in poi:
        scale, offset = 0.1, 1.0
    elif poi.startswith('SF_'):
        scale, offset = 1.0, 0.0
    else:
        raise ValueError(f"Unknown POI type: {poi}")

    central_values = np.array([el[1] * scale + offset for el in ydata], dtype=float)
    error_up = np.array([(el[2] - el[1]) * scale for el in ydata], dtype=float)
    error_down = np.array([(el[1] - el[0]) * scale for el in ydata], dtype=float)

    # mask NaN entries so missing bins don't get drawn
    mask = np.isfinite(central_values) & np.isfinite(error_up) & np.isfinite(error_down)

    ax.errorbar(
        xax[mask] + horizontal_shift,
        central_values[mask],
        yerr=[error_down[mask], error_up[mask]],
        fmt='o',
        linestyle='none',
        capsize=5,
        **kwargs
    )

    if len(xax) > 0:
        ax.set_xlim((-0.5, np.max(xax) + 0.5))
    if 'jms' in poi:
        ax.set_ylim((0.9, 1.1))
    elif 'jmr' in poi:
        ax.set_ylim((0.8, 1.2))
    # SF_tp* can sit well away from 1 in this measurement -> let matplotlib autoscale y

    if primary_xticklabels is not None:
        ax.set_xticks(ticks=xax, labels=primary_xticklabels, rotation=60, ha='right')

    if secondary_xticklabels is not None:
        for xval, label in secondary_xticklabels.items():
            ax.text(xval, -0.18, label, ha='left', va='top',
                    transform=ax.get_xaxis_transform())

    ax.axhline(y=1, color='black', linestyle='--', linewidth=1)
    ax.set_ylabel('Scale factor')
    ax.grid()
    fig.tight_layout()
    return fig, ax


if __name__=='__main__':

    impactfiles = sys.argv[1:]
    pois = ['tp2jms', 'tp2jmr', 'tp1jms', 'tp1jmr', 'tp3jms', 'tp3jmr']
    # tagger-score efficiency SFs, read from the "POIs" block of the impacts json
    pois += ['SF_tp2', 'SF_tp3', 'SF_tp1']

    data = {}
    years = []
    wps = []
    ptbins = []
    for impactfile in impactfiles:
        info = parse_impacts_file_name(impactfile)
        year = info['year']
        if year not in years:
            years.append(year)
        wp = info['wp']
        if wp not in wps:
            wps.append(wp)
        ptbin = info['bin']
        if ptbin not in ptbins:
            ptbins.append(ptbin)
        if year not in data:
            data[year] = {}
        if wp not in data[year]:
            data[year][wp] = {}
        if ptbin not in data[year][wp]:
            data[year][wp][ptbin] = {}

        params = load_impacts_json(impactfile)
        for poi in pois:
            if poi not in params:
                print(f'WARNING: parameter {poi} not found in {impactfile}.')
                continue
            data[year][wp][ptbin][poi] = params[poi]

    # sort pt bins
    ptmins = [float(ptbin.split('to')[0].replace('pt', '')) for ptbin in ptbins]
    ptmaxs = [float(ptbin.split('to')[1].replace('pt', '')) for ptbin in ptbins]
    ptbins_sorted = [(ptbin, ptmin, ptmax) for ptbin, ptmin, ptmax in zip(ptbins, ptmins, ptmaxs)]
    ptbins_sorted = sorted(ptbins_sorted, key=lambda x: (x[2], -x[1]))
    ptbins = [ptbin[0] for ptbin in ptbins_sorted]

    # sort years
    years_sorted = sorted([y.replace('pre','xxx').replace('post', 'zzz') for y in years])
    years = [y.replace('xxx', 'pre').replace('zzz', 'post') for y in years_sorted]

    # build one common x-axis layout for all plots
    slots = []
    ptticklabels = []
    yearticklabels = {}
    for year in years:
        yearticklabels[len(slots)] = year
        for ptbin in ptbins:
            slots.append((year, ptbin))
            ptticklabels.append('pT ' + ptbin.replace('pt', '').replace('to', ' - '))

    xpositions = np.arange(len(slots), dtype=float)

    # first kind: one WP per plot
    for poi in pois:
        for wp in wps:
            ydata = []
            for year, ptbin in slots:
                if year in data and wp in data[year] and ptbin in data[year][wp] and poi in data[year][wp][ptbin]:
                    lo, val, hi = data[year][wp][ptbin][poi]
                    ydata.append((lo, val , hi ))   # shift only central+interval consistently
                else:
                    ydata.append((np.nan, np.nan, np.nan))

            fig, ax = plot_scalefactors(
                ydata,
                poi=poi,
                xpositions=xpositions,
                primary_xticklabels=ptticklabels,
                secondary_xticklabels=yearticklabels
            )
            wplabel = wp.replace('to', ' - ')
            ax.text(0.02, 1.02, f'POI: {poi}, WP: {wplabel}', transform=ax.transAxes)
            fig.tight_layout()
            fig.savefig(f'test_{poi}_{wp}.png')

    # second kind: all WPs on one plot, same x-axis layout
    for poi in pois:
        fig = None
        ax = None

        cmap = plt.get_cmap('cool')
        cvals = np.linspace(0, 1, num=len(wps), endpoint=True)
        colors = {wp: cmap(cvals[cidx]) for cidx, wp in enumerate(wps)}

        spacing = 0.7
        xpositions = np.arange(len(slots), dtype=float) * spacing

        shifts = np.linspace(-0.10, 0.10, num=len(wps), endpoint=True)
        hshifts = {wp: shifts[idx] for idx, wp in enumerate(wps)}

        for wp in wps:
            ydata = []
            for year, ptbin in slots:
                if year in data and wp in data[year] and ptbin in data[year][wp] and poi in data[year][wp][ptbin]:
                    lo, val, hi = data[year][wp][ptbin][poi]
                    ydata.append((lo , val , hi ))
                else:
                    ydata.append((np.nan, np.nan, np.nan))

            wplabel = 'WP: ' + wp.replace('to', ' - ')
            fig, ax = plot_scalefactors(
                ydata,
                poi=poi,
                fig=fig,
                ax=ax,
                xpositions=xpositions,
                label=wplabel,
                color=colors[wp],
                horizontal_shift=hshifts[wp],
                primary_xticklabels=ptticklabels,
                secondary_xticklabels=yearticklabels
            )

        ax.legend(ncols=len(wps))
        ax.text(0.02, 1.02, f'POI: {poi}', transform=ax.transAxes)
        fig.tight_layout()
        fig.savefig(f'test_{poi}.png')