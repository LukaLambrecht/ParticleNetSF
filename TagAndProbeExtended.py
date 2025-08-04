# Implementation of the physics model for Combine
# for this tag-and-probe measurement.


from HiggsAnalysis.CombinedLimit.PhysicsModel import PhysicsModel


class TagAndProbeExtended(PhysicsModel):

    def __init__(self):
        super(PhysicsModel, self).__init__()

    def _getProcessCategory(self, process):
        # get the correct category for a given process.
        # note: the categories for this model are defined by the user,
        #       on the command line, with the text2workspace command,
        #       see setPhysicsOptions below.
        # note: for this specific application, the processes and the categories
        #       are both "tp3", "tp2", "tp1", and "other", so this function
        #       just returns the process itself.
        ret = None
        if self.DC.isSignal[process] or True:
            # (note: not sure why the line above is needed, it seems to be always true)
            for cat in self._categories:
                if cat in process:
                    ret = cat
                    break
        return ret

    def setPhysicsOptions(self, physOptions):
        # set the categories for this model
        # (+ other physics options passed down to the base class).
        # note: the categories for this model are defined by the user,
        #       on the command line, with the text2workspace command,
        #       in the form "--PO categories=<comma-separated list of categories>".
        # note: in this specific application, the categories are set to
        #       "tp3", "tp2", "tp1", and "other".
        for po in physOptions[:]:
            if po.startswith("categories="):
                physOptions.remove(po)
                self._categories = po.replace("categories=", "").split(",")
        super(TagAndProbeExtended, self).setPhysicsOptions(physOptions)

    def doParametersOfInterest(self):
        """Create POI and other parameters, and define the POI set."""
        # the parameters of interest are set to:
        # - SF_<category> for each category
        #   -> these are used to scale the "pass" category
        #      (see also below in getYieldScale).
        # other defined parameters are:
        # - fail_scale_<category> for each category
        #   -> these are used to scale the "fail" category
        #      (see also below in getYieldScale).

        # define the SF_<category> variables and set them as the POIs
        # note: these variables have no physical meaning yet, they are just declared.
        pois = []
        for cat in self._categories:
            self.modelBuilder.doVar("SF_%s[1.,0.,3.]" % cat)
            pois.append('SF_%s' % cat)
        self.modelBuilder.doSet("POI", ','.join(pois))

        # (what is this?)
        exp_pass = {}
        exp_fail = {}
        for b in self.DC.bins:
            for p in self.DC.exp[b].keys():
                cat = self._getProcessCategory(p)
                if cat is None:
                    continue
                if 'pass' in b:
                    exp_pass[cat] = self.DC.exp[b][p]
                elif 'fail' in b:
                    exp_fail[cat] = self.DC.exp[b][p]

        # define the fail_scale_<category> variables as a function of SF_<category>:
        # fail_scale_<cat> = (pass_exp + fail_exp - pass_exp * SF_<cat>) / fail_exp.
        # the reason for this definition is the following:
        # - the pass category is scaled with SF, so this gives:
        #     pass_exp * SF.
        # - the fail category is scaled with fail_scale, so this gives:
        #     fail_exp * fail_scale
        #     = pass_exp + fail_exp - pass_exp * SF
        #     = fail_exp + (1 - SF) * pass_exp
        # - so the sum of pass and fail remains the same, only the ratio changes.
        for cat in self._categories:
            expr = '({pass_exp}+{fail_exp}-({pass_exp}*@0))/{fail_exp}'.format(
                     pass_exp=exp_pass[cat], fail_exp=exp_fail[cat])
            expr = f'max(0., {expr})' # (safety to avoid negative yields)
            self.modelBuilder.factory_('expr::fail_scale_{cat}("{expr}", SF_{cat})'.format(cat=cat, expr=expr))


    def getYieldScale(self, bin, process):
        "Return the name of a RooAbsReal to scale this yield by or the two special values 1 and 0 (don't scale, and set to zero)"
        # define how to scale the processes.
        # as mentioned above, processes in the "pass" category are scaled with SF_<category>,
        # while processes in the "fail" category are scaled with fail_scale_<category>.
        if self.DC.isSignal[process] or True :
            cat = self._getProcessCategory(process)
            if cat is None:
                return 1
            if 'pass' in bin:
                return 'SF_%s' % cat
            else:
                return 'fail_scale_%s' % cat
        else:
            return 1


tagAndProbe = TagAndProbeExtended()
