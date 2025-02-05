/*
 * ReplicatedParty.cpp
 *
 */

#include "ShareParty.h"

#include "Thread.h"
#include "ShareThread.h"
#include "SemiPrep.h"
#include "Networking/Server.h"
#include "Networking/CryptoPlayer.h"
#include "Tools/ezOptionParser.h"
#include "Tools/benchmarking.h"
#include "Tools/NetworkOptions.h"
#include "Protocols/fake-stuff.h"

#include "ShareThread.hpp"
#include "RepPrep.hpp"
#include "Protocols/Replicated.hpp"
#include "Protocols/ReplicatedPrep.hpp"
#include "Protocols/MaliciousRepMC.hpp"
#include "Protocols/fake-stuff.hpp"

namespace GC
{

template<class T>
ShareParty<T>* ShareParty<T>::singleton = 0;

template<class T>
ShareParty<T>::ShareParty(int argc, const char** argv, int default_batch_size) :
        ThreadMaster<T>(online_opts), online_opts(opt, argc, argv,
                default_batch_size)
{
    if (singleton)
        throw runtime_error("there can only be one");
    singleton = this;

    NetworkOptionsWithNumber network_opts(opt, argc, argv,
            T::dishonest_majority ? 2 : 3, T::dishonest_majority);
    if (T::dishonest_majority)
        opt.add(
                "", // Default.
                0, // Required?
                0, // Number of args expected.
                0, // Delimiter if expecting multiple args.
                "Use encrypted channels.", // Help description.
                "-e", // Flag token.
                "--encrypted" // Flag token.
        );
    else
        opt.add(
                "", // Default.
                0, // Required?
                0, // Number of args expected.
                0, // Delimiter if expecting multiple args.
                "Unencrypted communication.", // Help description.
                "-u", // Flag token.
                "--unencrypted" // Flag token.
        );
    opt.add(
            "", // Default.
            0, // Required?
            0, // Number of args expected.
            0, // Delimiter if expecting multiple args.
            "Check opening by communication instead of hashing.", // Help description.
            "-c", // Flag token.
            "--communication" // Flag token.
    );
    online_opts.finalize(opt, argc, argv);
    OnlineOptions::singleton = online_opts;
    this->progname = online_opts.progname;
    int my_num = online_opts.playerno;

    if (T::dishonest_majority)
        this->machine.use_encryption = opt.get("-e")->isSet;
    else
        this->machine.use_encryption = not opt.get("-u")->isSet;

    this->machine.more_comm_less_comp = opt.get("-c")->isSet;

    T::out.activate(my_num == 0 or online_opts.interactive);

    if (not this->machine.use_encryption and not T::dishonest_majority)
        insecure("unencrypted communication");

    Server* server = network_opts.start_networking(this->N, my_num);

    if (online_opts.live_prep)
        if (T::needs_ot)
        {
            Player* P;
            if (this->machine.use_encryption)
                P = new CryptoPlayer(this->N, 0xFFFF);
            else
                P = new PlainPlayer(this->N, 0xFFFF);
            for (int i = 0; i < this->machine.nthreads; i++)
                this->machine.ot_setups.push_back({*P, true});
            delete P;
        }

    try
    {
        gf2n _;
        read_mac_keys(get_prep_dir(network_opts.nplayers, 128, 128), this->N,
                this->mac_key, _);
    }
    catch (exception& e)
    {
        SeededPRNG G;
        this->mac_key.randomize(G);
    }

    this->run();

    this->machine.write_memory(this->N.my_num());

    if (server)
        delete server;
}

template<class T>
Thread<T>* ShareParty<T>::new_thread(int i)
{
    return new StandaloneShareThread<T>(i, *this);
}

template<class T>
void ShareParty<T>::post_run()
{
    DataPositions usage;
    for (auto thread : this->threads)
        usage.increase(dynamic_cast<StandaloneShareThread<T>*>(thread)->usage);
    usage.print_cost();
}

}
