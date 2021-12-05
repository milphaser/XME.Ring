# XME.Ring

 Article: Distributed Ring-based Mutual Exclusion with Failure Recovery
 https://dl.acm.org/citation.cfm?doid=3407982.3408014
 Article: Distributed Ring-based Mutual Exclusion with Graceful Degradation
 https://www.researchgate.net/publication/356786353_Distributed_Ring-based_Mutual_Exclusion_with_Graceful_Degradation

 IDE: Embarcadero C++ Builder 11
 Platform: Windows/IA-32 & Intel64

 <<< Working Schemes Flow of Evolution >>>

 Scheme 1. Failure Recovery without reconfiguration
   Assumes that no any changes in the system configuration are made.
   After the faulty process is restored, all other processes resume normal operation
   from the state they were in at the time of failure and the ME token is restored.
   In doing so, strict compliance with the ME1 and ME2 requirements is ensured.
   This modified distributed ring-based mutual exclusion algorithm without communication ring
   reconfiguration designated as Mx1ME allows distributed system recovery from multiple faults.

 Scheme 2. Failure Recovery with one-way reconfiguration
   Supposes exclusion of the faulty processes from system configuration
   and hence leads to system graceful degradation., i.e. "Ring Shrinking".
   Such a working scheme is appropriate in the case of absence of spare processes.

 Scheme 3. Failure Recovery with two-way reconfiguration
   Supposes both exclusion of the faulty processes and inclusion (injection) of faultless spare processes
   as replacement of faulty ones. It is possible as well to "stretch the ring" by adding new processes in it.
   So the communication ring acquires the property of elasticity.
