#ifndef IWEIGHTPFOTOOL_H
#define IWEIGHTPFOTOOL_H

/** Simple class to retrieve PFO for jets and met in the two possible configurations we provide */

#include "AsgTools/IAsgTool.h"

#include "xAODPFlow/PFOContainer.h"
#include "PFlowUtils/PFODefs.h"

namespace CP {

  class IWeightPFOTool : public virtual asg::IAsgTool {

    /** Declare the interface that the class provides */
    ASG_TOOL_INTERFACE( CP::IWeightPFOTool )
      
    public:

    /** given a PFO, extract weight */
    virtual StatusCode fillWeight( const xAOD::PFO& cpfo, float& weight ) const = 0;

  };

}
#endif
