#include "../include/WSEML.hpp"
#include "../include/misc.hpp"
#include "../include/pointers.hpp"
#include "../include/dllconfig.hpp"
#include "../include/parser.hpp"
#include "../include/helpFunc.hpp"

namespace wseml {

    /// [':=', dest:ref, data:ref, N:ps]bc
    WSEML assignment(const WSEML& Args) {
        List* ArgsList = dynamic_cast<List*>(const_cast<Object*>(Args.getRawObject()));
        WSEML* obj = extractObj(ArgsList->find("obj"));
        WSEML* stack = extractObj(ArgsList->find("stack"));
        WSEML* frm = extractObj(ArgsList->find("frm"));
        WSEML* cmd = extractObj(ArgsList->find("cmd"));

        List* cmdList = dynamic_cast<List*>(cmd->getRawObject());
        WSEML& dataRef = cmdList->find("data");
        WSEML& destRef = cmdList->find("dest");

        List* proc = dynamic_cast<List*>(obj->getRawObject());
        List* tables = dynamic_cast<List*>(proc->find("tables").getRawObject());
        WSEML& data_o = proc->find("data");
        List* data = dynamic_cast<List*>(data_o.getRawObject());
        List* uref = dynamic_cast<List*>(tables->find("uref").getRawObject());
        List* readList = dynamic_cast<List*>(uref->find("read").getRawObject());
        List* writeList = dynamic_cast<List*>(uref->find("write").getRawObject());
        List* stackList = dynamic_cast<List*>(stack->getRawObject());
        List* stackInfo = dynamic_cast<List*>(stackList->find("info").getRawObject());
        WSEML* wfrm = &stackInfo->find("wfrm");

        std::string readDll = dynamic_cast<ByteString*>(readList->find("dllName").getRawObject())->get();
        std::string readFunc = dynamic_cast<ByteString*>(readList->find("funcName").getRawObject())->get();
        std::string writeDll = dynamic_cast<ByteString*>(writeList->find("dllName").getRawObject())->get();
        std::string writeFunc = dynamic_cast<ByteString*>(writeList->find("funcName").getRawObject())->get();

        std::string curStackKey = dynamic_cast<ByteString*>(stack->getContainingPair()->getKey().getRawObject())->get();
        std::string curFrmKey = dynamic_cast<ByteString*>(frm->getContainingPair()->getKey().getRawObject())->get();
        data->append(
            &data_o,
            parse(
                "{1:$[t:r]ps, 2:$[t:k, k:stck]ps, 3:$[t:k, k:" + curStackKey + "]ps, 4:$[t:k, k:" + curFrmKey +
                "]ps, 5:$[t:k, k:ip]ps, 6:$[t:i, i:-1]ps}"
            ),
            WSEML("assign_tmpPtr")
        );
        data->append(&data_o, *cmd, WSEML("assign_cmdCopy"));
        data->append(&data_o, NULLOBJ, WSEML("assign_tmp"));

        WSEML equivKey = createEquiv(stack, wfrm, frm, "assignment", "16");

        std::string procStr = getAddrStr(obj);
        WSEML ipRef = parse("{type:d, 1:$[type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:assign_tmpPtr]ps]ptr]ref}");
        WSEML newPs = parse("{type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 2:$[t:k, k:data]ps, 3:$[t:k, k:assign_cmdCopy]ps, 4$[t:k, k:N]ps]ptr}");
        ipRef.setSemanticType(WSEML("ref"));
        newPs.setSemanticType(WSEML("ref"));

        std::list<Pair> tmp;
        WSEML refArgs(tmp);
        List* args = dynamic_cast<List*>(const_cast<Object*>(refArgs.getRawObject()));
        args->append(&refArgs, ipRef, WSEML("ref"));
        args->append(&refArgs, newPs, WSEML("data"));
        WSEML res;
        if (cmdList->back() != NULLOBJ) {
            res = callFunc(writeDll.c_str(), writeFunc.c_str(), refArgs);
            if (res == WSEML("stopped"))
                return res;
        }

        changeCommand(stackList, equivKey, "17");

        args->find("ref") = dataRef;
        args->find("data") = parse("{type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:assign_tmp]ps]ptr}");
        res = callFunc(readDll.c_str(), readFunc.c_str(), refArgs);
        if (res == WSEML("stopped"))
            return res;

        changeCommand(stackList, equivKey, "18");
        args->find("ref") = destRef;
        res = callFunc(writeDll.c_str(), writeFunc.c_str(), refArgs);
        if (res == WSEML("stopped"))
            return res;

        WSEML DataKeys = parse("{1:assign_tmp, 2:assign_cmdCopy, 3:assign_tmpPtr}");
        clear(stackList, data, wfrm, equivKey, DataKeys);

        return WSEML("completed");
    }

    /// [`+', R, O1, O2, N]bc
    WSEML addition(const WSEML& Args) {
        List* ArgsList = dynamic_cast<List*>(const_cast<Object*>(Args.getRawObject()));
        WSEML* obj = extractObj(ArgsList->find("obj"));
        WSEML* stack = extractObj(ArgsList->find("stack"));
        WSEML* frm = extractObj(ArgsList->find("frm"));
        WSEML* cmd = extractObj(ArgsList->find("cmd"));

        List* cmdList = dynamic_cast<List*>(cmd->getRawObject());
        WSEML& R = cmdList->find("R");
        WSEML& O1 = cmdList->find("O1");
        WSEML& O2 = cmdList->find("O2");

        List* proc = dynamic_cast<List*>(obj->getRawObject());
        List* tables = dynamic_cast<List*>(proc->find("tables").getRawObject());
        WSEML& data_o = proc->find("data");
        List* data = dynamic_cast<List*>(data_o.getRawObject());
        List* uref = dynamic_cast<List*>(tables->find("uref").getRawObject());
        List* readList = dynamic_cast<List*>(uref->find("read").getRawObject());
        List* writeList = dynamic_cast<List*>(uref->find("write").getRawObject());
        List* stackList = dynamic_cast<List*>(stack->getRawObject());
        List* stackInfo = dynamic_cast<List*>(stackList->find("info").getRawObject());
        WSEML* wfrm = &stackInfo->find("wfrm");

        std::string readDll = dynamic_cast<ByteString*>(readList->find("dllName").getRawObject())->get();
        std::string readFunc = dynamic_cast<ByteString*>(readList->find("funcName").getRawObject())->get();
        std::string writeDll = dynamic_cast<ByteString*>(writeList->find("dllName").getRawObject())->get();
        std::string writeFunc = dynamic_cast<ByteString*>(writeList->find("funcName").getRawObject())->get();

        std::string curStackKey = dynamic_cast<ByteString*>(stack->getContainingPair()->getKey().getRawObject())->get();
        std::string curFrmKey = dynamic_cast<ByteString*>(frm->getContainingPair()->getKey().getRawObject())->get();
        data->append(
            &data_o,
            parse(
                "{1:$[t:r]ps, 2:$[t:k, k:stck]ps, 3:$[t:k, k:" + curStackKey + "]ps, 4:$[t:k, k:" + curFrmKey +
                "]ps, 5:$[t:k, k:ip]ps, 6:$[t:i, i:-1]ps}"
            ),
            WSEML("add_tmpPtr")
        );
        data->append(&data_o, NULLOBJ, WSEML("add_O1"));
        data->append(&data_o, NULLOBJ, WSEML("add_O2"));
        data->append(&data_o, *cmd, WSEML("add_cmdCopy"));
        data->append(&data_o, NULLOBJ, WSEML("add_res"));

        WSEML equivKey = createEquiv(stack, wfrm, frm, "addition", "19");

        std::string procStr = getAddrStr(obj);
        WSEML ipRef = parse("{type:d, 1:$[type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:add_tmpPtr]ps]ptr]ref}");
        WSEML newPs = parse("{type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 2:$[t:k, k:data]ps, 3:$[t:k, k:add_cmdCopy]ps, 4$[t:k, k:N]ps]ptr}");
        ipRef.setSemanticType(WSEML("ref"));
        newPs.setSemanticType(WSEML("ref"));

        std::list<Pair> tmp;
        WSEML refArgs(tmp);
        List* args = dynamic_cast<List*>(const_cast<Object*>(refArgs.getRawObject()));
        args->append(&refArgs, ipRef, WSEML("ref"));
        args->append(&refArgs, newPs, WSEML("data"));
        WSEML res;
        if (cmdList->back() != NULLOBJ) {
            res = callFunc(writeDll.c_str(), writeFunc.c_str(), refArgs);
            if (res == WSEML("stopped"))
                return res;
        }

        changeCommand(stackList, equivKey, "20");

        args->find("ref") = O1;
        args->find("data") = parse("{type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:add_O1]ps]ptr}");

        res = callFunc(readDll.c_str(), readFunc.c_str(), refArgs);
        if (res == WSEML("stopped"))
            return res;

        changeCommand(stackList, equivKey, "21");

        args->find("ref") = O2;
        args->find("data") = parse("{type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:add_O2]ps]ptr}");
        res = callFunc(readDll.c_str(), readDll.c_str(), refArgs);
        if (res == WSEML("stopped"))
            return res;

        WSEML OpArgs = parse(
            "{O1:$[type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:add_O1]ps]ptr]ref, O2:$[type:d, 1:$[comp:$[addr:" +
            procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:add_O2]ps]ptr]ref, res:$[type:d, 1:$[comp:$[addr:" + procStr +
            "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:add_res]ps]ptr]ref}"
        );
        safeSum(OpArgs);

        changeCommand(stackList, equivKey, "23");

        args->find("ref") = R;
        args->find("data") = parse("{type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:add_res]ps]ptr}");
        res = callFunc(writeDll.c_str(), writeDll.c_str(), refArgs);
        if (res == WSEML("stopped"))
            return res;

        WSEML DataKeys = parse("{1:add_O1, 2:add_O2, 3:add_cmdCopy, 4:add_res, 5:add_tmpPtr}");
        clear(stackList, data, wfrm, equivKey, DataKeys);

        return WSEML("completed");
    }

    /// [`-', R, O1, O2, N]bc
    WSEML subtraction(const WSEML& Args) {
        List* ArgsList = dynamic_cast<List*>(const_cast<Object*>(Args.getRawObject()));
        WSEML* obj = extractObj(ArgsList->find("obj"));
        WSEML* stack = extractObj(ArgsList->find("stack"));
        WSEML* frm = extractObj(ArgsList->find("frm"));
        WSEML* cmd = extractObj(ArgsList->find("cmd"));

        List* cmdList = dynamic_cast<List*>(cmd->getRawObject());
        WSEML& R = cmdList->find("R");
        WSEML& O1 = cmdList->find("O1");
        WSEML& O2 = cmdList->find("O2");

        List* proc = dynamic_cast<List*>(obj->getRawObject());
        List* tables = dynamic_cast<List*>(proc->find("tables").getRawObject());
        WSEML& data_o = proc->find("data");
        List* data = dynamic_cast<List*>(data_o.getRawObject());
        List* uref = dynamic_cast<List*>(tables->find("uref").getRawObject());
        List* readList = dynamic_cast<List*>(uref->find("read").getRawObject());
        List* writeList = dynamic_cast<List*>(uref->find("write").getRawObject());
        List* stackList = dynamic_cast<List*>(stack->getRawObject());
        List* stackInfo = dynamic_cast<List*>(stackList->find("info").getRawObject());
        WSEML* wfrm = &stackInfo->find("wfrm");

        std::string readDll = dynamic_cast<ByteString*>(readList->find("dllName").getRawObject())->get();
        std::string readFunc = dynamic_cast<ByteString*>(readList->find("funcName").getRawObject())->get();
        std::string writeDll = dynamic_cast<ByteString*>(writeList->find("dllName").getRawObject())->get();
        std::string writeFunc = dynamic_cast<ByteString*>(writeList->find("funcName").getRawObject())->get();

        std::string curStackKey = dynamic_cast<ByteString*>(stack->getContainingPair()->getKey().getRawObject())->get();
        std::string curFrmKey = dynamic_cast<ByteString*>(frm->getContainingPair()->getKey().getRawObject())->get();
        data->append(
            &data_o,
            parse(
                "{1:$[t:r]ps, 2:$[t:k, k:stck]ps, 3:$[t:k, k:" + curStackKey + "]ps, 4:$[t:k, k:" + curFrmKey +
                "]ps, 5:$[t:k, k:ip]ps, 6:$[t:i, i:-1]ps}"
            ),
            WSEML("sub_tmpPtr")
        );
        data->append(&data_o, NULLOBJ, WSEML("sub_O1"));
        data->append(&data_o, NULLOBJ, WSEML("sub_O2"));
        data->append(&data_o, *cmd, WSEML("sub_cmdCopy"));
        data->append(&data_o, NULLOBJ, WSEML("sub_res"));

        WSEML equivKey = createEquiv(stack, wfrm, frm, "subtraction", "19");

        std::string procStr = getAddrStr(obj);
        WSEML ipRef = parse("{type:d, 1:$[type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:sub_tmpPtr]ps]ptr]ref}");
        WSEML newPs = parse("{type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 2:$[t:k, k:data]ps, 3:$[t:k, k:sub_cmdCopy]ps, 4$[t:k, k:N]ps]ptr}");
        ipRef.setSemanticType(WSEML("ref"));
        newPs.setSemanticType(WSEML("ref"));

        std::list<Pair> tmp;
        WSEML refArgs(tmp);
        List* args = dynamic_cast<List*>(const_cast<Object*>(refArgs.getRawObject()));
        args->append(&refArgs, ipRef, WSEML("ref"));
        args->append(&refArgs, newPs, WSEML("data"));
        WSEML res;
        if (cmdList->back() != NULLOBJ) {
            res = callFunc(writeDll.c_str(), writeFunc.c_str(), refArgs);
            if (res == WSEML("stopped"))
                return res;
        }

        changeCommand(stackList, equivKey, "20");
        args->find("ref") = O1;
        args->find("data") = parse("{type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:sub_O1]ps]ptr}");

        res = callFunc(readDll.c_str(), readFunc.c_str(), refArgs);
        if (res == WSEML("stopped"))
            return res;

        changeCommand(stackList, equivKey, "21");

        args->find("ref") = O2;
        args->find("data") = parse("{type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:sub_O2]ps]ptr}");
        res = callFunc(readDll.c_str(), readDll.c_str(), refArgs);
        if (res == WSEML("stopped"))
            return res;

        WSEML OpArgs = parse(
            "{O1:$[type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:sub_O1]ps]ptr]ref, O2:$[type:d, 1:$[comp:$[addr:" +
            procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:sub_O2]ps]ptr]ref, res:$[type:d, 1:$[comp:$[addr:" + procStr +
            "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:sub_res]ps]ptr]ref}"
        );
        safeSub(OpArgs);

        changeCommand(stackList, equivKey, "23");

        args->find("ref") = R;
        args->find("data") = parse("{type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:sub_res]ps]ptr}");
        res = callFunc(writeDll.c_str(), writeDll.c_str(), refArgs);
        if (res == WSEML("stopped"))
            return res;

        WSEML DataKeys = parse("{1:sub_O1, 2:sub_O2, 3:sub_cmdCopy, 4:sub_res, 5:sub_tmpPtr}");
        clear(stackList, data, wfrm, equivKey, DataKeys);

        return WSEML("completed");
    }

    /// [`*', R, O1, O2, N]bc
    WSEML multiplication(const WSEML& Args) {
        List* ArgsList = dynamic_cast<List*>(const_cast<Object*>(Args.getRawObject()));
        WSEML* obj = extractObj(ArgsList->find("obj"));
        WSEML* stack = extractObj(ArgsList->find("stack"));
        WSEML* frm = extractObj(ArgsList->find("frm"));
        WSEML* cmd = extractObj(ArgsList->find("cmd"));

        List* cmdList = dynamic_cast<List*>(cmd->getRawObject());
        WSEML& R = cmdList->find("R");
        WSEML& O1 = cmdList->find("O1");
        WSEML& O2 = cmdList->find("O2");

        List* proc = dynamic_cast<List*>(obj->getRawObject());
        List* tables = dynamic_cast<List*>(proc->find("tables").getRawObject());
        WSEML& data_o = proc->find("data");
        List* data = dynamic_cast<List*>(data_o.getRawObject());
        List* uref = dynamic_cast<List*>(tables->find("uref").getRawObject());
        List* readList = dynamic_cast<List*>(uref->find("read").getRawObject());
        List* writeList = dynamic_cast<List*>(uref->find("write").getRawObject());
        List* stackList = dynamic_cast<List*>(stack->getRawObject());
        List* stackInfo = dynamic_cast<List*>(stackList->find("info").getRawObject());
        WSEML* wfrm = &stackInfo->find("wfrm");

        std::string readDll = dynamic_cast<ByteString*>(readList->find("dllName").getRawObject())->get();
        std::string readFunc = dynamic_cast<ByteString*>(readList->find("funcName").getRawObject())->get();
        std::string writeDll = dynamic_cast<ByteString*>(writeList->find("dllName").getRawObject())->get();
        std::string writeFunc = dynamic_cast<ByteString*>(writeList->find("funcName").getRawObject())->get();

        std::string curStackKey = dynamic_cast<ByteString*>(stack->getContainingPair()->getKey().getRawObject())->get();
        std::string curFrmKey = dynamic_cast<ByteString*>(frm->getContainingPair()->getKey().getRawObject())->get();
        data->append(
            &data_o,
            parse(
                "{1:$[t:r]ps, 2:$[t:k, k:stck]ps, 3:$[t:k, k:" + curStackKey + "]ps, 4:$[t:k, k:" + curFrmKey +
                "]ps, 5:$[t:k, k:ip]ps, 6:$[t:i, i:-1]ps}"
            ),
            WSEML("mult_tmpPtr")
        );
        data->append(&data_o, NULLOBJ, WSEML("mult_O1"));
        data->append(&data_o, NULLOBJ, WSEML("mult_O2"));
        data->append(&data_o, *cmd, WSEML("mult_cmdCopy"));
        data->append(&data_o, NULLOBJ, WSEML("mult_res"));

        WSEML equivKey = createEquiv(stack, wfrm, frm, "multiplication", "19");

        std::string procStr = getAddrStr(obj);
        WSEML ipRef = parse("{type:d, 1:$[type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:mult_tmpPtr]ps]ptr]ref}");
        WSEML newPs = parse("{type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 2:$[t:k, k:data]ps, 3:$[t:k, k:mult_cmdCopy]ps, 4$[t:k, k:N]ps]ptr}");
        ipRef.setSemanticType(WSEML("ref"));
        newPs.setSemanticType(WSEML("ref"));

        std::list<Pair> tmp;
        WSEML refArgs(tmp);
        List* args = dynamic_cast<List*>(const_cast<Object*>(refArgs.getRawObject()));
        args->append(&refArgs, ipRef, WSEML("ref"));
        args->append(&refArgs, newPs, WSEML("data"));
        WSEML res;
        if (cmdList->back() != NULLOBJ) {
            res = callFunc(writeDll.c_str(), writeFunc.c_str(), refArgs);
            if (res == WSEML("stopped"))
                return res;
        }

        changeCommand(stackList, equivKey, "20");

        args->find("ref") = O1;
        args->find("data") = parse("{type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:mult_O1]ps]ptr}");

        res = callFunc(readDll.c_str(), readFunc.c_str(), refArgs);
        if (res == WSEML("stopped"))
            return res;

        changeCommand(stackList, equivKey, "21");

        args->find("ref") = O2;
        args->find("data") = parse("{type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:mult_O2]ps]ptr}");
        res = callFunc(readDll.c_str(), readDll.c_str(), refArgs);
        if (res == WSEML("stopped"))
            return res;

        WSEML OpArgs = parse(
            "{O1:$[type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:mult_O1]ps]ptr]ref, O2:$[type:d, 1:$[comp:$[addr:" +
            procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:mult_O2]ps]ptr]ref, res:$[type:d, 1:$[comp:$[addr:" + procStr +
            "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:mult_res]ps]ptr]ref}"
        );
        safeMult(OpArgs);

        changeCommand(stackList, equivKey, "23");

        args->find("ref") = R;
        args->find("data") = parse("{type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:mult_res]ps]ptr}");
        res = callFunc(writeDll.c_str(), writeDll.c_str(), refArgs);
        if (res == WSEML("stopped"))
            return res;

        WSEML DataKeys = parse("{1:mult_O1, 2:mult_O2, 3:mult_cmdCopy, 4:mult_res, 5:mult_tmpPtr}");
        clear(stackList, data, wfrm, equivKey, DataKeys);

        return WSEML("completed");
    }

    /// [`/', R, O1, O2, N]bc
    WSEML division(const WSEML& Args) {
        List* ArgsList = dynamic_cast<List*>(const_cast<Object*>(Args.getRawObject()));
        WSEML* obj = extractObj(ArgsList->find("obj"));
        WSEML* stack = extractObj(ArgsList->find("stack"));
        WSEML* frm = extractObj(ArgsList->find("frm"));
        WSEML* cmd = extractObj(ArgsList->find("cmd"));

        List* cmdList = dynamic_cast<List*>(cmd->getRawObject());
        WSEML& R = cmdList->find("R");
        WSEML& O1 = cmdList->find("O1");
        WSEML& O2 = cmdList->find("O2");

        List* proc = dynamic_cast<List*>(obj->getRawObject());
        List* tables = dynamic_cast<List*>(proc->find("tables").getRawObject());
        WSEML& data_o = proc->find("data");
        List* data = dynamic_cast<List*>(data_o.getRawObject());
        List* uref = dynamic_cast<List*>(tables->find("uref").getRawObject());
        List* readList = dynamic_cast<List*>(uref->find("read").getRawObject());
        List* writeList = dynamic_cast<List*>(uref->find("write").getRawObject());
        List* stackList = dynamic_cast<List*>(stack->getRawObject());
        List* stackInfo = dynamic_cast<List*>(stackList->find("info").getRawObject());
        WSEML* wfrm = &stackInfo->find("wfrm");

        std::string readDll = dynamic_cast<ByteString*>(readList->find("dllName").getRawObject())->get();
        std::string readFunc = dynamic_cast<ByteString*>(readList->find("funcName").getRawObject())->get();
        std::string writeDll = dynamic_cast<ByteString*>(writeList->find("dllName").getRawObject())->get();
        std::string writeFunc = dynamic_cast<ByteString*>(writeList->find("funcName").getRawObject())->get();

        std::string curStackKey = dynamic_cast<ByteString*>(stack->getContainingPair()->getKey().getRawObject())->get();
        std::string curFrmKey = dynamic_cast<ByteString*>(frm->getContainingPair()->getKey().getRawObject())->get();
        data->append(
            &data_o,
            parse(
                "{1:$[t:r]ps, 2:$[t:k, k:stck]ps, 3:$[t:k, k:" + curStackKey + "]ps, 4:$[t:k, k:" + curFrmKey +
                "]ps, 5:$[t:k, k:ip]ps, 6:$[t:i, i:-1]ps}"
            ),
            WSEML("div_tmpPtr")
        );
        data->append(&data_o, NULLOBJ, WSEML("div_O1"));
        data->append(&data_o, NULLOBJ, WSEML("div_O2"));
        data->append(&data_o, *cmd, WSEML("div_cmdCopy"));
        data->append(&data_o, NULLOBJ, WSEML("div_res"));

        WSEML equivKey = createEquiv(stack, wfrm, frm, "division", "19");

        std::string procStr = getAddrStr(obj);
        WSEML ipRef = parse("{type:d, 1:$[type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:div_tmpPtr]ps]ptr]ref}");
        WSEML newPs = parse("{type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 2:$[t:k, k:data]ps, 3:$[t:k, k:div_cmdCopy]ps, 4$[t:k, k:N]ps]ptr}");
        ipRef.setSemanticType(WSEML("ref"));
        newPs.setSemanticType(WSEML("ref"));

        std::list<Pair> tmp;
        WSEML refArgs(tmp);
        List* args = dynamic_cast<List*>(const_cast<Object*>(refArgs.getRawObject()));
        args->append(&refArgs, ipRef, WSEML("ref"));
        args->append(&refArgs, newPs, WSEML("data"));
        WSEML res;
        if (cmdList->back() != NULLOBJ) {
            res = callFunc(writeDll.c_str(), writeFunc.c_str(), refArgs);
            if (res == WSEML("stopped"))
                return res;
        }

        changeCommand(stackList, equivKey, "20");

        args->find("ref") = O1;
        args->find("data") = parse("{type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:div_O1]ps]ptr}");

        res = callFunc(readDll.c_str(), readFunc.c_str(), refArgs);
        if (res == WSEML("stopped"))
            return res;

        changeCommand(stackList, equivKey, "21");

        args->find("ref") = O2;
        args->find("data") = parse("{type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:div_O2]ps]ptr}");
        res = callFunc(readDll.c_str(), readDll.c_str(), refArgs);
        if (res == WSEML("stopped"))
            return res;

        WSEML OpArgs = parse(
            "{O1:$[type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:div_O1]ps]ptr]ref, O2:$[type:d, 1:$[comp:$[addr:" +
            procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:div_O2]ps]ptr]ref, res:$[type:d, 1:$[comp:$[addr:" + procStr +
            "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:div_res]ps]ptr]ref}"
        );
        safeDiv(OpArgs);

        changeCommand(stackList, equivKey, "23");

        args->find("ref") = R;
        args->find("data") = parse("{type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:div_res]ps]ptr}");
        res = callFunc(writeDll.c_str(), writeDll.c_str(), refArgs);
        if (res == WSEML("stopped"))
            return res;

        WSEML DataKeys = parse("{1:div_O1, 2:div_O2, 3:div_cmdCopy, 4:div_res, 5:div_tmpPtr}");
        clear(stackList, data, wfrm, equivKey, DataKeys);

        return WSEML("completed");
    }

    /// [`%', R, O1, O2, N]bc
    WSEML remainder(const WSEML& Args) {
        List* ArgsList = dynamic_cast<List*>(const_cast<Object*>(Args.getRawObject()));
        WSEML* obj = extractObj(ArgsList->find("obj"));
        WSEML* stack = extractObj(ArgsList->find("stack"));
        WSEML* frm = extractObj(ArgsList->find("frm"));
        WSEML* cmd = extractObj(ArgsList->find("cmd"));

        List* cmdList = dynamic_cast<List*>(cmd->getRawObject());
        WSEML& R = cmdList->find("R");
        WSEML& O1 = cmdList->find("O1");
        WSEML& O2 = cmdList->find("O2");

        List* proc = dynamic_cast<List*>(obj->getRawObject());
        List* tables = dynamic_cast<List*>(proc->find("tables").getRawObject());
        WSEML& data_o = proc->find("data");
        List* data = dynamic_cast<List*>(data_o.getRawObject());
        List* uref = dynamic_cast<List*>(tables->find("uref").getRawObject());
        List* readList = dynamic_cast<List*>(uref->find("read").getRawObject());
        List* writeList = dynamic_cast<List*>(uref->find("write").getRawObject());
        List* stackList = dynamic_cast<List*>(stack->getRawObject());
        List* stackInfo = dynamic_cast<List*>(stackList->find("info").getRawObject());
        WSEML* wfrm = &stackInfo->find("wfrm");

        std::string readDll = dynamic_cast<ByteString*>(readList->find("dllName").getRawObject())->get();
        std::string readFunc = dynamic_cast<ByteString*>(readList->find("funcName").getRawObject())->get();
        std::string writeDll = dynamic_cast<ByteString*>(writeList->find("dllName").getRawObject())->get();
        std::string writeFunc = dynamic_cast<ByteString*>(writeList->find("funcName").getRawObject())->get();

        std::string curStackKey = dynamic_cast<ByteString*>(stack->getContainingPair()->getKey().getRawObject())->get();
        std::string curFrmKey = dynamic_cast<ByteString*>(frm->getContainingPair()->getKey().getRawObject())->get();
        data->append(
            &data_o,
            parse(
                "{1:$[t:r]ps, 2:$[t:k, k:stck]ps, 3:$[t:k, k:" + curStackKey + "]ps, 4:$[t:k, k:" + curFrmKey +
                "]ps, 5:$[t:k, k:ip]ps, 6:$[t:i, i:-1]ps}"
            ),
            WSEML("mod_tmpPtr")
        );
        data->append(&data_o, NULLOBJ, WSEML("mod_O1"));
        data->append(&data_o, NULLOBJ, WSEML("mod_O2"));
        data->append(&data_o, *cmd, WSEML("mod_cmdCopy"));
        data->append(&data_o, NULLOBJ, WSEML("mod_res"));

        WSEML equivKey = createEquiv(stack, wfrm, frm, "remainder", "19");

        std::string procStr = getAddrStr(obj);
        WSEML ipRef = parse("{type:d, 1:$[type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:mod_tmpPtr]ps]ptr]ref}");
        WSEML newPs = parse("{type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 2:$[t:k, k:data]ps, 3:$[t:k, k:mod_cmdCopy]ps, 4$[t:k, k:N]ps]ptr}");
        ipRef.setSemanticType(WSEML("ref"));
        newPs.setSemanticType(WSEML("ref"));

        std::list<Pair> tmp;
        WSEML refArgs(tmp);
        List* args = dynamic_cast<List*>(const_cast<Object*>(refArgs.getRawObject()));
        args->append(&refArgs, ipRef, WSEML("ref"));
        args->append(&refArgs, newPs, WSEML("data"));
        WSEML res;
        if (cmdList->back() != NULLOBJ) {
            res = callFunc(writeDll.c_str(), writeFunc.c_str(), refArgs);
            if (res == WSEML("stopped"))
                return res;
        }

        changeCommand(stackList, equivKey, "20");

        args->find("ref") = O1;
        args->find("data") = parse("{type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:mod_O1]ps]ptr}");

        res = callFunc(readDll.c_str(), readFunc.c_str(), refArgs);
        if (res == WSEML("stopped"))
            return res;

        changeCommand(stackList, equivKey, "21");

        args->find("ref") = O2;
        args->find("data") = parse("{type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:mod_O2]ps]ptr}");
        res = callFunc(readDll.c_str(), readDll.c_str(), refArgs);
        if (res == WSEML("stopped"))
            return res;

        WSEML OpArgs = parse(
            "{O1:$[type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:mod_O1]ps]ptr]ref, O2:$[type:d, 1:$[comp:$[addr:" +
            procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:mod_O2]ps]ptr]ref, res:$[type:d, 1:$[comp:$[addr:" + procStr +
            "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:mod_res]ps]ptr]ref}"
        );
        safeMod(OpArgs);

        changeCommand(stackList, equivKey, "23");

        args->find("ref") = R;
        args->find("data") = parse("{type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:mod_res]ps]ptr}");
        res = callFunc(writeDll.c_str(), writeDll.c_str(), refArgs);
        if (res == WSEML("stopped"))
            return res;

        WSEML DataKeys = parse("{1:mod_O1, 2:mod_O2, 3:mod_cmdCopy, 4:mod_res, 5:mod_tmpPtr}");
        clear(stackList, data, wfrm, equivKey, DataKeys);

        return WSEML("completed");
    }

    /// [`^', R, O1, O2, N]bc
    WSEML power(const WSEML& Args) {
        List* ArgsList = dynamic_cast<List*>(const_cast<Object*>(Args.getRawObject()));
        WSEML* obj = extractObj(ArgsList->find("obj"));
        WSEML* stack = extractObj(ArgsList->find("stack"));
        WSEML* frm = extractObj(ArgsList->find("frm"));
        WSEML* cmd = extractObj(ArgsList->find("cmd"));

        List* cmdList = dynamic_cast<List*>(cmd->getRawObject());
        WSEML& R = cmdList->find("R");
        WSEML& O1 = cmdList->find("O1");
        WSEML& O2 = cmdList->find("O2");

        List* proc = dynamic_cast<List*>(obj->getRawObject());
        List* tables = dynamic_cast<List*>(proc->find("tables").getRawObject());
        WSEML& data_o = proc->find("data");
        List* data = dynamic_cast<List*>(data_o.getRawObject());
        List* uref = dynamic_cast<List*>(tables->find("uref").getRawObject());
        List* readList = dynamic_cast<List*>(uref->find("read").getRawObject());
        List* writeList = dynamic_cast<List*>(uref->find("write").getRawObject());
        List* stackList = dynamic_cast<List*>(stack->getRawObject());
        List* stackInfo = dynamic_cast<List*>(stackList->find("info").getRawObject());
        WSEML* wfrm = &stackInfo->find("wfrm");

        std::string readDll = dynamic_cast<ByteString*>(readList->find("dllName").getRawObject())->get();
        std::string readFunc = dynamic_cast<ByteString*>(readList->find("funcName").getRawObject())->get();
        std::string writeDll = dynamic_cast<ByteString*>(writeList->find("dllName").getRawObject())->get();
        std::string writeFunc = dynamic_cast<ByteString*>(writeList->find("funcName").getRawObject())->get();

        std::string curStackKey = dynamic_cast<ByteString*>(stack->getContainingPair()->getKey().getRawObject())->get();
        std::string curFrmKey = dynamic_cast<ByteString*>(frm->getContainingPair()->getKey().getRawObject())->get();
        data->append(
            &data_o,
            parse(
                "{1:$[t:r]ps, 2:$[t:k, k:stck]ps, 3:$[t:k, k:" + curStackKey + "]ps, 4:$[t:k, k:" + curFrmKey +
                "]ps, 5:$[t:k, k:ip]ps, 6:$[t:i, i:-1]ps}"
            ),
            WSEML("pow_tmpPtr")
        );
        data->append(&data_o, NULLOBJ, WSEML("pow_O1"));
        data->append(&data_o, NULLOBJ, WSEML("pow_O2"));
        data->append(&data_o, *cmd, WSEML("pow_cmdCopy"));
        data->append(&data_o, NULLOBJ, WSEML("pow_res"));

        WSEML equivKey = createEquiv(stack, wfrm, frm, "power", "19");

        std::string procStr = getAddrStr(obj);
        WSEML ipRef = parse("{type:d, 1:$[type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:pow_tmpPtr]ps]ptr]ref}");
        WSEML newPs = parse("{type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 2:$[t:k, k:data]ps, 3:$[t:k, k:pow_cmdCopy]ps, 4$[t:k, k:N]ps]ptr}");
        ipRef.setSemanticType(WSEML("ref"));
        newPs.setSemanticType(WSEML("ref"));

        std::list<Pair> tmp;
        WSEML refArgs(tmp);
        List* args = dynamic_cast<List*>(const_cast<Object*>(refArgs.getRawObject()));
        args->append(&refArgs, ipRef, WSEML("ref"));
        args->append(&refArgs, newPs, WSEML("data"));
        WSEML res;
        if (cmdList->back() != NULLOBJ) {
            res = callFunc(writeDll.c_str(), writeFunc.c_str(), refArgs);
            if (res == WSEML("stopped"))
                return res;
        }

        changeCommand(stackList, equivKey, "20");

        args->find("ref") = O1;
        args->find("data") = parse("{type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:pow_O1]ps]ptr}");

        res = callFunc(readDll.c_str(), readFunc.c_str(), refArgs);
        if (res == WSEML("stopped"))
            return res;

        changeCommand(stackList, equivKey, "21");

        args->find("ref") = O2;
        args->find("data") = parse("{type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:pow_O2]ps]ptr}");
        res = callFunc(readDll.c_str(), readDll.c_str(), refArgs);
        if (res == WSEML("stopped"))
            return res;

        WSEML OpArgs = parse(
            "{O1:$[type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:pow_O1]ps]ptr]ref, O2:$[type:d, 1:$[comp:$[addr:" +
            procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:pow_O2]ps]ptr]ref, res:$[type:d, 1:$[comp:$[addr:" + procStr +
            "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:pow_res]ps]ptr]ref}"
        );
        safePow(OpArgs);

        changeCommand(stackList, equivKey, "23");

        args->find("ref") = R;
        args->find("data") = parse("{type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:pow_res]ps]ptr}");
        res = callFunc(writeDll.c_str(), writeDll.c_str(), refArgs);
        if (res == WSEML("stopped"))
            return res;

        WSEML DataKeys = parse("{1:pow_O1, 2:pow_O2, 3:pow_cmdCopy, 4:pow_res, 5:pow_tmpPtr}");
        clear(stackList, data, wfrm, equivKey, DataKeys);

        return WSEML("completed");
    }

    /// [`.', R, O1, O2, N]bc
    WSEML concatenate(const WSEML& Args) {
        List* ArgsList = dynamic_cast<List*>(const_cast<Object*>(Args.getRawObject()));
        WSEML* obj = extractObj(ArgsList->find("obj"));
        WSEML* stack = extractObj(ArgsList->find("stack"));
        WSEML* frm = extractObj(ArgsList->find("frm"));
        WSEML* cmd = extractObj(ArgsList->find("cmd"));

        List* cmdList = dynamic_cast<List*>(cmd->getRawObject());
        WSEML& R = cmdList->find("R");
        WSEML& O1 = cmdList->find("O1");
        WSEML& O2 = cmdList->find("O2");

        List* proc = dynamic_cast<List*>(obj->getRawObject());
        List* tables = dynamic_cast<List*>(proc->find("tables").getRawObject());
        WSEML& data_o = proc->find("data");
        List* data = dynamic_cast<List*>(data_o.getRawObject());
        List* uref = dynamic_cast<List*>(tables->find("uref").getRawObject());
        List* readList = dynamic_cast<List*>(uref->find("read").getRawObject());
        List* writeList = dynamic_cast<List*>(uref->find("write").getRawObject());
        List* stackList = dynamic_cast<List*>(stack->getRawObject());
        List* stackInfo = dynamic_cast<List*>(stackList->find("info").getRawObject());
        WSEML* wfrm = &stackInfo->find("wfrm");

        std::string readDll = dynamic_cast<ByteString*>(readList->find("dllName").getRawObject())->get();
        std::string readFunc = dynamic_cast<ByteString*>(readList->find("funcName").getRawObject())->get();
        std::string writeDll = dynamic_cast<ByteString*>(writeList->find("dllName").getRawObject())->get();
        std::string writeFunc = dynamic_cast<ByteString*>(writeList->find("funcName").getRawObject())->get();

        std::string curStackKey = dynamic_cast<ByteString*>(stack->getContainingPair()->getKey().getRawObject())->get();
        std::string curFrmKey = dynamic_cast<ByteString*>(frm->getContainingPair()->getKey().getRawObject())->get();
        data->append(
            &data_o,
            parse(
                "{1:$[t:r]ps, 2:$[t:k, k:stck]ps, 3:$[t:k, k:" + curStackKey + "]ps, 4:$[t:k, k:" + curFrmKey +
                "]ps, 5:$[t:k, k:ip]ps, 6:$[t:i, i:-1]ps}"
            ),
            WSEML("concat_tmpPtr")
        );
        data->append(&data_o, NULLOBJ, WSEML("concat_O1"));
        data->append(&data_o, NULLOBJ, WSEML("concat_O2"));
        data->append(&data_o, *cmd, WSEML("concat_cmdCopy"));
        data->append(&data_o, NULLOBJ, WSEML("concat_res"));

        WSEML equivKey = createEquiv(stack, wfrm, frm, "concatenation", "19");

        std::string procStr = getAddrStr(obj);
        WSEML ipRef = parse("{type:d, 1:$[type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:concat_tmpPtr]ps]ptr]ref}");
        WSEML newPs = parse("{type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 2:$[t:k, k:data]ps, 3:$[t:k, k:concat_cmdCopy]ps, 4$[t:k, k:N]ps]ptr}");
        ipRef.setSemanticType(WSEML("ref"));
        newPs.setSemanticType(WSEML("ref"));

        std::list<Pair> tmp;
        WSEML refArgs(tmp);
        List* args = dynamic_cast<List*>(const_cast<Object*>(refArgs.getRawObject()));
        args->append(&refArgs, ipRef, WSEML("ref"));
        args->append(&refArgs, newPs, WSEML("data"));
        WSEML res;
        if (cmdList->back() != NULLOBJ) {
            res = callFunc(writeDll.c_str(), writeFunc.c_str(), refArgs);
            if (res == WSEML("stopped"))
                return res;
        }

        changeCommand(stackList, equivKey, "20");

        args->find("ref") = O1;
        args->find("data") = parse("{type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:concat_O1]ps]ptr}");

        res = callFunc(readDll.c_str(), readFunc.c_str(), refArgs);
        if (res == WSEML("stopped"))
            return res;

        changeCommand(stackList, equivKey, "21");

        args->find("ref") = O2;
        args->find("data") = parse("{type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:concat_O2]ps]ptr}");
        res = callFunc(readDll.c_str(), readDll.c_str(), refArgs);
        if (res == WSEML("stopped"))
            return res;

        WSEML OpArgs = parse(
            "{O1:$[type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:concat_O1]ps]ptr]ref, O2:$[type:d, 1:$[comp:$[addr:" +
            procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:concat_O2]ps]ptr]ref, res:$[type:d, 1:$[comp:$[addr:" + procStr +
            "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:concat_res]ps]ptr]ref}"
        );
        safeConcat(OpArgs);

        changeCommand(stackList, equivKey, "23");

        args->find("ref") = R;
        args->find("data") = parse("{type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:concat_res]ps]ptr}");
        res = callFunc(writeDll.c_str(), writeDll.c_str(), refArgs);
        if (res == WSEML("stopped"))
            return res;

        WSEML DataKeys = parse("{1:concat_O1, 2:concat_O2, 3:concat_cmdCopy, 4:concat_res, 5:concat_tmpPtr}");
        clear(stackList, data, wfrm, equivKey, DataKeys);

        return WSEML("completed");
    }

    /// [‘=’, R, O1, O2, N]bc
    WSEML isEq(const WSEML& Args) {
        List* ArgsList = dynamic_cast<List*>(const_cast<Object*>(Args.getRawObject()));
        WSEML* obj = extractObj(ArgsList->find("obj"));
        WSEML* stack = extractObj(ArgsList->find("stack"));
        WSEML* frm = extractObj(ArgsList->find("frm"));
        WSEML* cmd = extractObj(ArgsList->find("cmd"));

        List* cmdList = dynamic_cast<List*>(cmd->getRawObject());
        WSEML& R = cmdList->find("R");
        WSEML& O1 = cmdList->find("O1");
        WSEML& O2 = cmdList->find("O2");

        List* proc = dynamic_cast<List*>(obj->getRawObject());
        List* tables = dynamic_cast<List*>(proc->find("tables").getRawObject());
        WSEML& data_o = proc->find("data");
        List* data = dynamic_cast<List*>(data_o.getRawObject());
        List* uref = dynamic_cast<List*>(tables->find("uref").getRawObject());
        List* readList = dynamic_cast<List*>(uref->find("read").getRawObject());
        List* writeList = dynamic_cast<List*>(uref->find("write").getRawObject());
        List* stackList = dynamic_cast<List*>(stack->getRawObject());
        List* stackInfo = dynamic_cast<List*>(stackList->find("info").getRawObject());
        WSEML* wfrm = &stackInfo->find("wfrm");

        std::string readDll = dynamic_cast<ByteString*>(readList->find("dllName").getRawObject())->get();
        std::string readFunc = dynamic_cast<ByteString*>(readList->find("funcName").getRawObject())->get();
        std::string writeDll = dynamic_cast<ByteString*>(writeList->find("dllName").getRawObject())->get();
        std::string writeFunc = dynamic_cast<ByteString*>(writeList->find("funcName").getRawObject())->get();

        std::string curStackKey = dynamic_cast<ByteString*>(stack->getContainingPair()->getKey().getRawObject())->get();
        std::string curFrmKey = dynamic_cast<ByteString*>(frm->getContainingPair()->getKey().getRawObject())->get();
        data->append(
            &data_o,
            parse(
                "{1:$[t:r]ps, 2:$[t:k, k:stck]ps, 3:$[t:k, k:" + curStackKey + "]ps, 4:$[t:k, k:" + curFrmKey +
                "]ps, 5:$[t:k, k:ip]ps, 6:$[t:i, i:-1]ps}"
            ),
            WSEML("eq_tmpPtr")
        );
        data->append(&data_o, NULLOBJ, WSEML("eq_O1"));
        data->append(&data_o, NULLOBJ, WSEML("eq_O2"));
        data->append(&data_o, *cmd, WSEML("eq_cmdCopy"));
        data->append(&data_o, NULLOBJ, WSEML("eq_res"));

        WSEML equivKey = createEquiv(stack, wfrm, frm, "isEq", "19");

        std::string procStr = getAddrStr(obj);
        WSEML ipRef = parse("{type:d, 1:$[type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:eq_tmpPtr]ps]ptr]ref}");
        WSEML newPs = parse("{type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 2:$[t:k, k:data]ps, 3:$[t:k, k:eq_cmdCopy]ps, 4$[t:k, k:N]ps]ptr}");
        ipRef.setSemanticType(WSEML("ref"));
        newPs.setSemanticType(WSEML("ref"));

        std::list<Pair> tmp;
        WSEML refArgs(tmp);
        List* args = dynamic_cast<List*>(const_cast<Object*>(refArgs.getRawObject()));
        args->append(&refArgs, ipRef, WSEML("ref"));
        args->append(&refArgs, newPs, WSEML("data"));
        WSEML res;
        if (cmdList->back() != NULLOBJ) {
            res = callFunc(writeDll.c_str(), writeFunc.c_str(), refArgs);
            if (res == WSEML("stopped"))
                return res;
        }

        changeCommand(stackList, equivKey, "20");

        args->find("ref") = O1;
        args->find("data") = parse("{type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:eq_O1]ps]ptr}");

        res = callFunc(readDll.c_str(), readFunc.c_str(), refArgs);
        if (res == WSEML("stopped"))
            return res;

        changeCommand(stackList, equivKey, "21");

        args->find("ref") = O2;
        args->find("data") = parse("{type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:eq_O2]ps]ptr}");
        res = callFunc(readDll.c_str(), readDll.c_str(), refArgs);
        if (res == WSEML("stopped"))
            return res;

        WSEML OpArgs = parse(
            "{O1:$[type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:eq_O1]ps]ptr]ref, O2:$[type:d, 1:$[comp:$[addr:" +
            procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:eq_O2]ps]ptr]ref, res:$[type:d, 1:$[comp:$[addr:" + procStr +
            "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:eq_res]ps]ptr]ref}"
        );
        safeEq(OpArgs);

        changeCommand(stackList, equivKey, "23");

        args->find("ref") = R;
        args->find("data") = parse("{type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:eq_res]ps]ptr}");
        res = callFunc(writeDll.c_str(), writeDll.c_str(), refArgs);
        if (res == WSEML("stopped"))
            return res;

        WSEML DataKeys = parse("{1:eq_O1, 2:eq_O2, 3:eq_cmdCopy, 4:eq_res, 5:eq_tmpPtr}");
        clear(stackList, data, wfrm, equivKey, DataKeys);

        return WSEML("completed");
    }

    /// [‘!=’, R, O1, O2, N]bc
    WSEML isNeq(const WSEML& Args) {
        List* ArgsList = dynamic_cast<List*>(const_cast<Object*>(Args.getRawObject()));
        WSEML* obj = extractObj(ArgsList->find("obj"));
        WSEML* stack = extractObj(ArgsList->find("stack"));
        WSEML* frm = extractObj(ArgsList->find("frm"));
        WSEML* cmd = extractObj(ArgsList->find("cmd"));

        List* cmdList = dynamic_cast<List*>(cmd->getRawObject());
        WSEML& R = cmdList->find("R");
        WSEML& O1 = cmdList->find("O1");
        WSEML& O2 = cmdList->find("O2");

        List* proc = dynamic_cast<List*>(obj->getRawObject());
        List* tables = dynamic_cast<List*>(proc->find("tables").getRawObject());
        WSEML& data_o = proc->find("data");
        List* data = dynamic_cast<List*>(data_o.getRawObject());
        List* uref = dynamic_cast<List*>(tables->find("uref").getRawObject());
        List* readList = dynamic_cast<List*>(uref->find("read").getRawObject());
        List* writeList = dynamic_cast<List*>(uref->find("write").getRawObject());
        List* stackList = dynamic_cast<List*>(stack->getRawObject());
        List* stackInfo = dynamic_cast<List*>(stackList->find("info").getRawObject());
        WSEML* wfrm = &stackInfo->find("wfrm");

        std::string readDll = dynamic_cast<ByteString*>(readList->find("dllName").getRawObject())->get();
        std::string readFunc = dynamic_cast<ByteString*>(readList->find("funcName").getRawObject())->get();
        std::string writeDll = dynamic_cast<ByteString*>(writeList->find("dllName").getRawObject())->get();
        std::string writeFunc = dynamic_cast<ByteString*>(writeList->find("funcName").getRawObject())->get();

        std::string curStackKey = dynamic_cast<ByteString*>(stack->getContainingPair()->getKey().getRawObject())->get();
        std::string curFrmKey = dynamic_cast<ByteString*>(frm->getContainingPair()->getKey().getRawObject())->get();
        data->append(
            &data_o,
            parse(
                "{1:$[t:r]ps, 2:$[t:k, k:stck]ps, 3:$[t:k, k:" + curStackKey + "]ps, 4:$[t:k, k:" + curFrmKey +
                "]ps, 5:$[t:k, k:ip]ps, 6:$[t:i, i:-1]ps}"
            ),
            WSEML("neq_tmpPtr")
        );
        data->append(&data_o, NULLOBJ, WSEML("neq_O1"));
        data->append(&data_o, NULLOBJ, WSEML("neq_O2"));
        data->append(&data_o, *cmd, WSEML("neq_cmdCopy"));
        data->append(&data_o, NULLOBJ, WSEML("neq_res"));

        WSEML equivKey = createEquiv(stack, wfrm, frm, "isNeq", "19");

        std::string procStr = getAddrStr(obj);
        WSEML ipRef = parse("{type:d, 1:$[type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:neq_tmpPtr]ps]ptr]ref}");
        WSEML newPs = parse("{type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 2:$[t:k, k:data]ps, 3:$[t:k, k:neq_cmdCopy]ps, 4$[t:k, k:N]ps]ptr}");
        ipRef.setSemanticType(WSEML("ref"));
        newPs.setSemanticType(WSEML("ref"));

        std::list<Pair> tmp;
        WSEML refArgs(tmp);
        List* args = dynamic_cast<List*>(const_cast<Object*>(refArgs.getRawObject()));
        args->append(&refArgs, ipRef, WSEML("ref"));
        args->append(&refArgs, newPs, WSEML("data"));
        WSEML res;
        if (cmdList->back() != NULLOBJ) {
            res = callFunc(writeDll.c_str(), writeFunc.c_str(), refArgs);
            if (res == WSEML("stopped"))
                return res;
        }

        changeCommand(stackList, equivKey, "20");

        args->find("ref") = O1;
        args->find("data") = parse("{type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:neq_O1]ps]ptr}");
        res = callFunc(readDll.c_str(), readFunc.c_str(), refArgs);
        if (res == WSEML("stopped"))
            return res;

        changeCommand(stackList, equivKey, "21");

        args->find("ref") = O2;
        args->find("data") = parse("{type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:neq_O2]ps]ptr}");
        res = callFunc(readDll.c_str(), readDll.c_str(), refArgs);
        if (res == WSEML("stopped"))
            return res;

        WSEML OpArgs = parse(
            "{O1:$[type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:neq_O1]ps]ptr]ref, O2:$[type:d, 1:$[comp:$[addr:" +
            procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:neq_O2]ps]ptr]ref, res:$[type:d, 1:$[comp:$[addr:" + procStr +
            "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:neq_res]ps]ptr]ref}"
        );
        safeNeq(OpArgs);

        changeCommand(stackList, equivKey, "23");

        args->find("ref") = R;
        args->find("data") = parse("{type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:neq_res]ps]ptr}");
        res = callFunc(writeDll.c_str(), writeDll.c_str(), refArgs);
        if (res == WSEML("stopped"))
            return res;

        WSEML DataKeys = parse("{1:neq_O1, 2:neq_O2, 3:neq_cmdCopy, 4:neq_res, 5:neq_tmpPtr}");
        clear(stackList, data, wfrm, equivKey, DataKeys);

        return WSEML("completed");
    }

    /// [‘<', R, O1, O2, N]bc
    WSEML isLess(const WSEML& Args) {
        List* ArgsList = dynamic_cast<List*>(const_cast<Object*>(Args.getRawObject()));
        WSEML* obj = extractObj(ArgsList->find("obj"));
        WSEML* stack = extractObj(ArgsList->find("stack"));
        WSEML* frm = extractObj(ArgsList->find("frm"));
        WSEML* cmd = extractObj(ArgsList->find("cmd"));

        List* cmdList = dynamic_cast<List*>(cmd->getRawObject());
        WSEML& R = cmdList->find("R");
        WSEML& O1 = cmdList->find("O1");
        WSEML& O2 = cmdList->find("O2");

        List* proc = dynamic_cast<List*>(obj->getRawObject());
        List* tables = dynamic_cast<List*>(proc->find("tables").getRawObject());
        WSEML& data_o = proc->find("data");
        List* data = dynamic_cast<List*>(data_o.getRawObject());
        List* uref = dynamic_cast<List*>(tables->find("uref").getRawObject());
        List* readList = dynamic_cast<List*>(uref->find("read").getRawObject());
        List* writeList = dynamic_cast<List*>(uref->find("write").getRawObject());
        List* stackList = dynamic_cast<List*>(stack->getRawObject());
        List* stackInfo = dynamic_cast<List*>(stackList->find("info").getRawObject());
        WSEML* wfrm = &stackInfo->find("wfrm");

        std::string readDll = dynamic_cast<ByteString*>(readList->find("dllName").getRawObject())->get();
        std::string readFunc = dynamic_cast<ByteString*>(readList->find("funcName").getRawObject())->get();
        std::string writeDll = dynamic_cast<ByteString*>(writeList->find("dllName").getRawObject())->get();
        std::string writeFunc = dynamic_cast<ByteString*>(writeList->find("funcName").getRawObject())->get();

        std::string curStackKey = dynamic_cast<ByteString*>(stack->getContainingPair()->getKey().getRawObject())->get();
        std::string curFrmKey = dynamic_cast<ByteString*>(frm->getContainingPair()->getKey().getRawObject())->get();
        data->append(
            &data_o,
            parse(
                "{1:$[t:r]ps, 2:$[t:k, k:stck]ps, 3:$[t:k, k:" + curStackKey + "]ps, 4:$[t:k, k:" + curFrmKey +
                "]ps, 5:$[t:k, k:ip]ps, 6:$[t:i, i:-1]ps}"
            ),
            WSEML("less_tmpPtr")
        );
        data->append(&data_o, NULLOBJ, WSEML("less_O1"));
        data->append(&data_o, NULLOBJ, WSEML("less_O2"));
        data->append(&data_o, *cmd, WSEML("less_cmdCopy"));
        data->append(&data_o, NULLOBJ, WSEML("less_res"));

        WSEML equivKey = createEquiv(stack, wfrm, frm, "isLess", "19");

        std::string procStr = getAddrStr(obj);
        WSEML ipRef = parse("{type:d, 1:$[type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:less_tmpPtr]ps]ptr]ref}");
        WSEML newPs = parse("{type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 2:$[t:k, k:data]ps, 3:$[t:k, k:less_cmdCopy]ps, 4$[t:k, k:N]ps]ptr}");
        ipRef.setSemanticType(WSEML("ref"));
        newPs.setSemanticType(WSEML("ref"));

        std::list<Pair> tmp;
        WSEML refArgs(tmp);
        List* args = dynamic_cast<List*>(const_cast<Object*>(refArgs.getRawObject()));
        args->append(&refArgs, ipRef, WSEML("ref"));
        args->append(&refArgs, newPs, WSEML("data"));
        WSEML res;
        if (cmdList->back() != NULLOBJ) {
            res = callFunc(writeDll.c_str(), writeFunc.c_str(), refArgs);
            if (res == WSEML("stopped"))
                return res;
        }

        changeCommand(stackList, equivKey, "20");

        args->find("ref") = O1;
        args->find("data") = parse("{type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:less_O1]ps]ptr}");
        res = callFunc(readDll.c_str(), readFunc.c_str(), refArgs);
        if (res == WSEML("stopped"))
            return res;

        changeCommand(stackList, equivKey, "21");

        args->find("ref") = O2;
        args->find("data") = parse("{type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:less_O2]ps]ptr}");
        res = callFunc(readDll.c_str(), readDll.c_str(), refArgs);
        if (res == WSEML("stopped"))
            return res;

        WSEML OpArgs = parse(
            "{O1:$[type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:less_O1]ps]ptr]ref, O2:$[type:d, 1:$[comp:$[addr:" +
            procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:less_O2]ps]ptr]ref, res:$[type:d, 1:$[comp:$[addr:" + procStr +
            "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:less_res]ps]ptr]ref}"
        );
        safeLess(OpArgs);

        changeCommand(stackList, equivKey, "23");

        args->find("ref") = R;
        args->find("data") = parse("{type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:less_res]ps]ptr}");
        res = callFunc(writeDll.c_str(), writeDll.c_str(), refArgs);
        if (res == WSEML("stopped"))
            return res;

        WSEML DataKeys = parse("{1:less_O1, 2:less_O2, 3:less_cmdCopy, 4:less_res, 5:less_tmpPtr}");
        clear(stackList, data, wfrm, equivKey, DataKeys);

        return WSEML("completed");
    }

    /// [‘>’, R, O1, O2, N]bc
    WSEML isGreater(const WSEML& Args) {
        List* ArgsList = dynamic_cast<List*>(const_cast<Object*>(Args.getRawObject()));
        WSEML* obj = extractObj(ArgsList->find("obj"));
        WSEML* stack = extractObj(ArgsList->find("stack"));
        WSEML* frm = extractObj(ArgsList->find("frm"));
        WSEML* cmd = extractObj(ArgsList->find("cmd"));

        List* cmdList = dynamic_cast<List*>(cmd->getRawObject());
        WSEML& R = cmdList->find("R");
        WSEML& O1 = cmdList->find("O1");
        WSEML& O2 = cmdList->find("O2");

        List* proc = dynamic_cast<List*>(obj->getRawObject());
        List* tables = dynamic_cast<List*>(proc->find("tables").getRawObject());
        WSEML& data_o = proc->find("data");
        List* data = dynamic_cast<List*>(data_o.getRawObject());
        List* uref = dynamic_cast<List*>(tables->find("uref").getRawObject());
        List* readList = dynamic_cast<List*>(uref->find("read").getRawObject());
        List* writeList = dynamic_cast<List*>(uref->find("write").getRawObject());
        List* stackList = dynamic_cast<List*>(stack->getRawObject());
        List* stackInfo = dynamic_cast<List*>(stackList->find("info").getRawObject());
        WSEML* wfrm = &stackInfo->find("wfrm");

        std::string readDll = dynamic_cast<ByteString*>(readList->find("dllName").getRawObject())->get();
        std::string readFunc = dynamic_cast<ByteString*>(readList->find("funcName").getRawObject())->get();
        std::string writeDll = dynamic_cast<ByteString*>(writeList->find("dllName").getRawObject())->get();
        std::string writeFunc = dynamic_cast<ByteString*>(writeList->find("funcName").getRawObject())->get();

        std::string curStackKey = dynamic_cast<ByteString*>(stack->getContainingPair()->getKey().getRawObject())->get();
        std::string curFrmKey = dynamic_cast<ByteString*>(frm->getContainingPair()->getKey().getRawObject())->get();
        data->append(
            &data_o,
            parse(
                "{1:$[t:r]ps, 2:$[t:k, k:stck]ps, 3:$[t:k, k:" + curStackKey + "]ps, 4:$[t:k, k:" + curFrmKey +
                "]ps, 5:$[t:k, k:ip]ps, 6:$[t:i, i:-1]ps}"
            ),
            WSEML("greater_tmpPtr")
        );
        data->append(&data_o, NULLOBJ, WSEML("greater_O1"));
        data->append(&data_o, NULLOBJ, WSEML("greater_O2"));
        data->append(&data_o, *cmd, WSEML("greater_cmdCopy"));
        data->append(&data_o, NULLOBJ, WSEML("greater_res"));

        WSEML equivKey = createEquiv(stack, wfrm, frm, "isGreater", "19");

        std::string procStr = getAddrStr(obj);
        WSEML ipRef = parse("{type:d, 1:$[type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:greater_tmpPtr]ps]ptr]ref}");
        WSEML newPs = parse("{type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 2:$[t:k, k:data]ps, 3:$[t:k, k:greater_cmdCopy]ps, 4$[t:k, k:N]ps]ptr}");
        ipRef.setSemanticType(WSEML("ref"));
        newPs.setSemanticType(WSEML("ref"));

        std::list<Pair> tmp;
        WSEML refArgs(tmp);
        List* args = dynamic_cast<List*>(const_cast<Object*>(refArgs.getRawObject()));
        args->append(&refArgs, ipRef, WSEML("ref"));
        args->append(&refArgs, newPs, WSEML("data"));
        WSEML res;
        if (cmdList->back() != NULLOBJ) {
            res = callFunc(writeDll.c_str(), writeFunc.c_str(), refArgs);
            if (res == WSEML("stopped"))
                return res;
        }

        changeCommand(stackList, equivKey, "20");

        args->find("ref") = O1;
        args->find("data") = parse("{type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:greater_O1]ps]ptr}");
        res = callFunc(readDll.c_str(), readFunc.c_str(), refArgs);
        if (res == WSEML("stopped"))
            return res;

        changeCommand(stackList, equivKey, "21");

        args->find("ref") = O2;
        args->find("data") = parse("{type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:greater_O2]ps]ptr}");
        res = callFunc(readDll.c_str(), readDll.c_str(), refArgs);
        if (res == WSEML("stopped"))
            return res;

        WSEML OpArgs = parse(
            "{O1:$[type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:greater_O1]ps]ptr]ref, O2:$[type:d, 1:$[comp:$[addr:" +
            procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:greater_O2]ps]ptr]ref, res:$[type:d, 1:$[comp:$[addr:" + procStr +
            "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:greater_res]ps]ptr]ref}"
        );
        safeGreater(OpArgs);

        changeCommand(stackList, equivKey, "23");

        args->find("ref") = R;
        args->find("data") = parse("{type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:greater_res]ps]ptr}");
        res = callFunc(writeDll.c_str(), writeDll.c_str(), refArgs);
        if (res == WSEML("stopped"))
            return res;

        WSEML DataKeys = parse("{1:greater_O1, 2:greater_O2, 3:greater_cmdCopy, 4:greater_res, 5:greater_tmpPtr}");
        clear(stackList, data, wfrm, equivKey, DataKeys);

        return WSEML("completed");
    }

    /// [‘<=’, R, O1, O2, N]bc
    WSEML isLeq(const WSEML& Args) {
        List* ArgsList = dynamic_cast<List*>(const_cast<Object*>(Args.getRawObject()));
        WSEML* obj = extractObj(ArgsList->find("obj"));
        WSEML* stack = extractObj(ArgsList->find("stack"));
        WSEML* frm = extractObj(ArgsList->find("frm"));
        WSEML* cmd = extractObj(ArgsList->find("cmd"));

        List* cmdList = dynamic_cast<List*>(cmd->getRawObject());
        WSEML& R = cmdList->find("R");
        WSEML& O1 = cmdList->find("O1");
        WSEML& O2 = cmdList->find("O2");

        List* proc = dynamic_cast<List*>(obj->getRawObject());
        List* tables = dynamic_cast<List*>(proc->find("tables").getRawObject());
        WSEML& data_o = proc->find("data");
        List* data = dynamic_cast<List*>(data_o.getRawObject());
        List* uref = dynamic_cast<List*>(tables->find("uref").getRawObject());
        List* readList = dynamic_cast<List*>(uref->find("read").getRawObject());
        List* writeList = dynamic_cast<List*>(uref->find("write").getRawObject());
        List* stackList = dynamic_cast<List*>(stack->getRawObject());
        List* stackInfo = dynamic_cast<List*>(stackList->find("info").getRawObject());
        WSEML* wfrm = &stackInfo->find("wfrm");

        std::string readDll = dynamic_cast<ByteString*>(readList->find("dllName").getRawObject())->get();
        std::string readFunc = dynamic_cast<ByteString*>(readList->find("funcName").getRawObject())->get();
        std::string writeDll = dynamic_cast<ByteString*>(writeList->find("dllName").getRawObject())->get();
        std::string writeFunc = dynamic_cast<ByteString*>(writeList->find("funcName").getRawObject())->get();

        std::string curStackKey = dynamic_cast<ByteString*>(stack->getContainingPair()->getKey().getRawObject())->get();
        std::string curFrmKey = dynamic_cast<ByteString*>(frm->getContainingPair()->getKey().getRawObject())->get();
        data->append(
            &data_o,
            parse(
                "{1:$[t:r]ps, 2:$[t:k, k:stck]ps, 3:$[t:k, k:" + curStackKey + "]ps, 4:$[t:k, k:" + curFrmKey +
                "]ps, 5:$[t:k, k:ip]ps, 6:$[t:i, i:-1]ps}"
            ),
            WSEML("leq_tmpPtr")
        );
        data->append(&data_o, NULLOBJ, WSEML("leq_O1"));
        data->append(&data_o, NULLOBJ, WSEML("leq_O2"));
        data->append(&data_o, *cmd, WSEML("leq_cmdCopy"));
        data->append(&data_o, NULLOBJ, WSEML("leq_res"));

        WSEML equivKey = createEquiv(stack, wfrm, frm, "isLeq", "19");

        std::string procStr = getAddrStr(obj);
        WSEML ipRef = parse("{type:d, 1:$[type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:leq_tmpPtr]ps]ptr]ref}");
        WSEML newPs = parse("{type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 2:$[t:k, k:data]ps, 3:$[t:k, k:leq_cmdCopy]ps, 4$[t:k, k:N]ps]ptr}");
        ipRef.setSemanticType(WSEML("ref"));
        newPs.setSemanticType(WSEML("ref"));

        std::list<Pair> tmp;
        WSEML refArgs(tmp);
        List* args = dynamic_cast<List*>(const_cast<Object*>(refArgs.getRawObject()));
        args->append(&refArgs, ipRef, WSEML("ref"));
        args->append(&refArgs, newPs, WSEML("data"));
        WSEML res;
        if (cmdList->back() != NULLOBJ) {
            res = callFunc(writeDll.c_str(), writeFunc.c_str(), refArgs);
            if (res == WSEML("stopped"))
                return res;
        }

        changeCommand(stackList, equivKey, "20");

        args->find("ref") = O1;
        args->find("data") = parse("{type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:leq_O1]ps]ptr}");
        res = callFunc(readDll.c_str(), readFunc.c_str(), refArgs);
        if (res == WSEML("stopped"))
            return res;

        changeCommand(stackList, equivKey, "21");

        args->find("ref") = O2;
        args->find("data") = parse("{type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:leq_O2]ps]ptr}");
        res = callFunc(readDll.c_str(), readDll.c_str(), refArgs);
        if (res == WSEML("stopped"))
            return res;

        WSEML OpArgs = parse(
            "{O1:$[type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:leq_O1]ps]ptr]ref, O2:$[type:d, 1:$[comp:$[addr:" +
            procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:leq_O2]ps]ptr]ref, res:$[type:d, 1:$[comp:$[addr:" + procStr +
            "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:leq_res]ps]ptr]ref}"
        );
        safeLeq(OpArgs);

        changeCommand(stackList, equivKey, "23");

        args->find("ref") = R;
        args->find("data") = parse("{type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:leq_res]ps]ptr}");
        res = callFunc(writeDll.c_str(), writeDll.c_str(), refArgs);
        if (res == WSEML("stopped"))
            return res;

        WSEML DataKeys = parse("{1:leq_O1, 2:leq_O2, 3:leq_cmdCopy, 4:leq_res, 5:leq_tmpPtr}");
        clear(stackList, data, wfrm, equivKey, DataKeys);

        return WSEML("completed");
    }

    /// [‘>=’, R, O1, O2, N]bc
    WSEML isGeq(const WSEML& Args) {
        List* ArgsList = dynamic_cast<List*>(const_cast<Object*>(Args.getRawObject()));
        WSEML* obj = extractObj(ArgsList->find("obj"));
        WSEML* stack = extractObj(ArgsList->find("stack"));
        WSEML* frm = extractObj(ArgsList->find("frm"));
        WSEML* cmd = extractObj(ArgsList->find("cmd"));

        List* cmdList = dynamic_cast<List*>(cmd->getRawObject());
        WSEML& R = cmdList->find("R");
        WSEML& O1 = cmdList->find("O1");
        WSEML& O2 = cmdList->find("O2");

        List* proc = dynamic_cast<List*>(obj->getRawObject());
        List* tables = dynamic_cast<List*>(proc->find("tables").getRawObject());
        WSEML& data_o = proc->find("data");
        List* data = dynamic_cast<List*>(data_o.getRawObject());
        List* uref = dynamic_cast<List*>(tables->find("uref").getRawObject());
        List* readList = dynamic_cast<List*>(uref->find("read").getRawObject());
        List* writeList = dynamic_cast<List*>(uref->find("write").getRawObject());
        List* stackList = dynamic_cast<List*>(stack->getRawObject());
        List* stackInfo = dynamic_cast<List*>(stackList->find("info").getRawObject());
        WSEML* wfrm = &stackInfo->find("wfrm");

        std::string readDll = dynamic_cast<ByteString*>(readList->find("dllName").getRawObject())->get();
        std::string readFunc = dynamic_cast<ByteString*>(readList->find("funcName").getRawObject())->get();
        std::string writeDll = dynamic_cast<ByteString*>(writeList->find("dllName").getRawObject())->get();
        std::string writeFunc = dynamic_cast<ByteString*>(writeList->find("funcName").getRawObject())->get();

        std::string curStackKey = dynamic_cast<ByteString*>(stack->getContainingPair()->getKey().getRawObject())->get();
        std::string curFrmKey = dynamic_cast<ByteString*>(frm->getContainingPair()->getKey().getRawObject())->get();
        data->append(
            &data_o,
            parse(
                "{1:$[t:r]ps, 2:$[t:k, k:stck]ps, 3:$[t:k, k:" + curStackKey + "]ps, 4:$[t:k, k:" + curFrmKey +
                "]ps, 5:$[t:k, k:ip]ps, 6:$[t:i, i:-1]ps}"
            ),
            WSEML("geq_tmpPtr")
        );
        data->append(&data_o, NULLOBJ, WSEML("geq_O1"));
        data->append(&data_o, NULLOBJ, WSEML("geq_O2"));
        data->append(&data_o, *cmd, WSEML("geq_cmdCopy"));
        data->append(&data_o, NULLOBJ, WSEML("geq_res"));

        WSEML equivKey = createEquiv(stack, wfrm, frm, "isGeq", "19");

        std::string procStr = getAddrStr(obj);
        WSEML ipRef = parse("{type:d, 1:$[type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:geq_tmpPtr]ps]ptr]ref}");
        WSEML newPs = parse("{type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 2:$[t:k, k:data]ps, 3:$[t:k, k:geq_cmdCopy]ps, 4$[t:k, k:N]ps]ptr}");
        ipRef.setSemanticType(WSEML("ref"));
        newPs.setSemanticType(WSEML("ref"));

        std::list<Pair> tmp;
        WSEML refArgs(tmp);
        List* args = dynamic_cast<List*>(const_cast<Object*>(refArgs.getRawObject()));
        args->append(&refArgs, ipRef, WSEML("ref"));
        args->append(&refArgs, newPs, WSEML("data"));
        WSEML res;
        if (cmdList->back() != NULLOBJ) {
            res = callFunc(writeDll.c_str(), writeFunc.c_str(), refArgs);
            if (res == WSEML("stopped"))
                return res;
        }

        changeCommand(stackList, equivKey, "20");

        args->find("ref") = O1;
        args->find("data") = parse("{type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:geq_O1]ps]ptr}");
        res = callFunc(readDll.c_str(), readFunc.c_str(), refArgs);
        if (res == WSEML("stopped"))
            return res;

        changeCommand(stackList, equivKey, "21");

        args->find("ref") = O2;
        args->find("data") = parse("{type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:geq_O2]ps]ptr}");
        res = callFunc(readDll.c_str(), readDll.c_str(), refArgs);
        if (res == WSEML("stopped"))
            return res;

        WSEML OpArgs = parse(
            "{O1:$[type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:geq_O1]ps]ptr]ref, O2:$[type:d, 1:$[comp:$[addr:" +
            procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:geq_O2]ps]ptr]ref, res:$[type:d, 1:$[comp:$[addr:" + procStr +
            "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:geq_res]ps]ptr]ref}"
        );
        safeGeq(OpArgs);

        changeCommand(stackList, equivKey, "23");

        args->find("ref") = R;
        args->find("data") = parse("{type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:geq_res]ps]ptr}");
        res = callFunc(writeDll.c_str(), writeDll.c_str(), refArgs);
        if (res == WSEML("stopped"))
            return res;

        WSEML DataKeys = parse("{1:geq_O1, 2:geq_O2, 3:geq_cmdCopy, 4:geq_res, 5:geq_tmpPtr}");
        clear(stackList, data, wfrm, equivKey, DataKeys);

        return WSEML("completed");
    }

    /// [`&&', R, O1, O2, N]bc
    WSEML logicAnd(const WSEML& Args) {
        List* ArgsList = dynamic_cast<List*>(const_cast<Object*>(Args.getRawObject()));
        WSEML* obj = extractObj(ArgsList->find("obj"));
        WSEML* stack = extractObj(ArgsList->find("stack"));
        WSEML* frm = extractObj(ArgsList->find("frm"));
        WSEML* cmd = extractObj(ArgsList->find("cmd"));

        List* cmdList = dynamic_cast<List*>(cmd->getRawObject());
        WSEML& R = cmdList->find("R");
        WSEML& O1 = cmdList->find("O1");
        WSEML& O2 = cmdList->find("O2");

        List* proc = dynamic_cast<List*>(obj->getRawObject());
        List* tables = dynamic_cast<List*>(proc->find("tables").getRawObject());
        WSEML& data_o = proc->find("data");
        List* data = dynamic_cast<List*>(data_o.getRawObject());
        List* uref = dynamic_cast<List*>(tables->find("uref").getRawObject());
        List* readList = dynamic_cast<List*>(uref->find("read").getRawObject());
        List* writeList = dynamic_cast<List*>(uref->find("write").getRawObject());
        List* stackList = dynamic_cast<List*>(stack->getRawObject());
        List* stackInfo = dynamic_cast<List*>(stackList->find("info").getRawObject());
        WSEML* wfrm = &stackInfo->find("wfrm");

        std::string readDll = dynamic_cast<ByteString*>(readList->find("dllName").getRawObject())->get();
        std::string readFunc = dynamic_cast<ByteString*>(readList->find("funcName").getRawObject())->get();
        std::string writeDll = dynamic_cast<ByteString*>(writeList->find("dllName").getRawObject())->get();
        std::string writeFunc = dynamic_cast<ByteString*>(writeList->find("funcName").getRawObject())->get();

        std::string curStackKey = dynamic_cast<ByteString*>(stack->getContainingPair()->getKey().getRawObject())->get();
        std::string curFrmKey = dynamic_cast<ByteString*>(frm->getContainingPair()->getKey().getRawObject())->get();
        data->append(
            &data_o,
            parse(
                "{1:$[t:r]ps, 2:$[t:k, k:stck]ps, 3:$[t:k, k:" + curStackKey + "]ps, 4:$[t:k, k:" + curFrmKey +
                "]ps, 5:$[t:k, k:ip]ps, 6:$[t:i, i:-1]ps}"
            ),
            WSEML("and_tmpPtr")
        );
        data->append(&data_o, NULLOBJ, WSEML("and_O1"));
        data->append(&data_o, NULLOBJ, WSEML("and_O2"));
        data->append(&data_o, *cmd, WSEML("and_cmdCopy"));
        data->append(&data_o, NULLOBJ, WSEML("and_res"));

        WSEML equivKey = createEquiv(stack, wfrm, frm, "logicAnd", "19");

        std::string procStr = getAddrStr(obj);
        WSEML ipRef = parse("{type:d, 1:$[type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:and_tmpPtr]ps]ptr]ref}");
        WSEML newPs = parse("{type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 2:$[t:k, k:data]ps, 3:$[t:k, k:and_cmdCopy]ps, 4$[t:k, k:N]ps]ptr}");
        ipRef.setSemanticType(WSEML("ref"));
        newPs.setSemanticType(WSEML("ref"));

        std::list<Pair> tmp;
        WSEML refArgs(tmp);
        List* args = dynamic_cast<List*>(const_cast<Object*>(refArgs.getRawObject()));
        args->append(&refArgs, ipRef, WSEML("ref"));
        args->append(&refArgs, newPs, WSEML("data"));
        WSEML res;
        if (cmdList->back() != NULLOBJ) {
            res = callFunc(writeDll.c_str(), writeFunc.c_str(), refArgs);
            if (res == WSEML("stopped"))
                return res;
        }

        changeCommand(stackList, equivKey, "20");

        args->find("ref") = O1;
        args->find("data") = parse("{type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:and_O1]ps]ptr}");
        res = callFunc(readDll.c_str(), readFunc.c_str(), refArgs);
        if (res == WSEML("stopped"))
            return res;

        changeCommand(stackList, equivKey, "21");

        args->find("ref") = O2;
        args->find("data") = parse("{type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:and_O2]ps]ptr}");
        res = callFunc(readDll.c_str(), readDll.c_str(), refArgs);
        if (res == WSEML("stopped"))
            return res;

        WSEML OpArgs = parse(
            "{O1:$[type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:and_O1]ps]ptr]ref, O2:$[type:d, 1:$[comp:$[addr:" +
            procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:and_O2]ps]ptr]ref, res:$[type:d, 1:$[comp:$[addr:" + procStr +
            "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:and_res]ps]ptr]ref}"
        );
        safeAnd(OpArgs);

        changeCommand(stackList, equivKey, "23");

        args->find("ref") = R;
        args->find("data") = parse("{type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:and_res]ps]ptr}");
        res = callFunc(writeDll.c_str(), writeDll.c_str(), refArgs);
        if (res == WSEML("stopped"))
            return res;

        WSEML DataKeys = parse("{1:and_O1, 2:and_O2, 3:and_cmdCopy, 4:and_res, 5:and_tmpPtr}");
        clear(stackList, data, wfrm, equivKey, DataKeys);

        return WSEML("completed");
    }

    /// [`||', R, O1, O2, N]bc
    WSEML logicOr(const WSEML& Args) {
        List* ArgsList = dynamic_cast<List*>(const_cast<Object*>(Args.getRawObject()));
        WSEML* obj = extractObj(ArgsList->find("obj"));
        WSEML* stack = extractObj(ArgsList->find("stack"));
        WSEML* frm = extractObj(ArgsList->find("frm"));
        WSEML* cmd = extractObj(ArgsList->find("cmd"));

        List* cmdList = dynamic_cast<List*>(cmd->getRawObject());
        WSEML& R = cmdList->find("R");
        WSEML& O1 = cmdList->find("O1");
        WSEML& O2 = cmdList->find("O2");

        List* proc = dynamic_cast<List*>(obj->getRawObject());
        List* tables = dynamic_cast<List*>(proc->find("tables").getRawObject());
        WSEML& data_o = proc->find("data");
        List* data = dynamic_cast<List*>(data_o.getRawObject());
        List* uref = dynamic_cast<List*>(tables->find("uref").getRawObject());
        List* readList = dynamic_cast<List*>(uref->find("read").getRawObject());
        List* writeList = dynamic_cast<List*>(uref->find("write").getRawObject());
        List* stackList = dynamic_cast<List*>(stack->getRawObject());
        List* stackInfo = dynamic_cast<List*>(stackList->find("info").getRawObject());
        WSEML* wfrm = &stackInfo->find("wfrm");

        std::string readDll = dynamic_cast<ByteString*>(readList->find("dllName").getRawObject())->get();
        std::string readFunc = dynamic_cast<ByteString*>(readList->find("funcName").getRawObject())->get();
        std::string writeDll = dynamic_cast<ByteString*>(writeList->find("dllName").getRawObject())->get();
        std::string writeFunc = dynamic_cast<ByteString*>(writeList->find("funcName").getRawObject())->get();

        std::string curStackKey = dynamic_cast<ByteString*>(stack->getContainingPair()->getKey().getRawObject())->get();
        std::string curFrmKey = dynamic_cast<ByteString*>(frm->getContainingPair()->getKey().getRawObject())->get();
        data->append(
            &data_o,
            parse(
                "{1:$[t:r]ps, 2:$[t:k, k:stck]ps, 3:$[t:k, k:" + curStackKey + "]ps, 4:$[t:k, k:" + curFrmKey +
                "]ps, 5:$[t:k, k:ip]ps, 6:$[t:i, i:-1]ps}"
            ),
            WSEML("or_tmpPtr")
        );
        data->append(&data_o, NULLOBJ, WSEML("or_O1"));
        data->append(&data_o, NULLOBJ, WSEML("or_O2"));
        data->append(&data_o, *cmd, WSEML("or_cmdCopy"));
        data->append(&data_o, NULLOBJ, WSEML("or_res"));

        WSEML equivKey = createEquiv(stack, wfrm, frm, "logicOr", "19");

        std::string procStr = getAddrStr(obj);
        WSEML ipRef = parse("{type:d, 1:$[type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:or_tmpPtr]ps]ptr]ref}");
        WSEML newPs = parse("{type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 2:$[t:k, k:data]ps, 3:$[t:k, k:or_cmdCopy]ps, 4$[t:k, k:N]ps]ptr}");
        ipRef.setSemanticType(WSEML("ref"));
        newPs.setSemanticType(WSEML("ref"));

        std::list<Pair> tmp;
        WSEML refArgs(tmp);
        List* args = dynamic_cast<List*>(const_cast<Object*>(refArgs.getRawObject()));
        args->append(&refArgs, ipRef, WSEML("ref"));
        args->append(&refArgs, newPs, WSEML("data"));
        WSEML res;
        if (cmdList->back() != NULLOBJ) {
            res = callFunc(writeDll.c_str(), writeFunc.c_str(), refArgs);
            if (res == WSEML("stopped"))
                return res;
        }

        changeCommand(stackList, equivKey, "20");

        args->find("ref") = O1;
        args->find("data") = parse("{type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:or_O1]ps]ptr}");
        res = callFunc(readDll.c_str(), readFunc.c_str(), refArgs);
        if (res == WSEML("stopped"))
            return res;

        changeCommand(stackList, equivKey, "21");

        args->find("ref") = O2;
        args->find("data") = parse("{type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:or_O2]ps]ptr}");
        res = callFunc(readDll.c_str(), readDll.c_str(), refArgs);
        if (res == WSEML("stopped"))
            return res;

        WSEML OpArgs = parse(
            "{O1:$[type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:or_O1]ps]ptr]ref, O2:$[type:d, 1:$[comp:$[addr:" +
            procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:or_O2]ps]ptr]ref, res:$[type:d, 1:$[comp:$[addr:" + procStr +
            "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:or_res]ps]ptr]ref}"
        );
        safeOr(OpArgs);

        changeCommand(stackList, equivKey, "23");

        args->find("ref") = R;
        args->find("data") = parse("{type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:or_res]ps]ptr}");
        res = callFunc(writeDll.c_str(), writeDll.c_str(), refArgs);
        if (res == WSEML("stopped"))
            return res;

        WSEML DataKeys = parse("{1:or_O1, 2:or_O2, 3:or_cmdCopy, 4:or_res, 5:or_tmpPtr}");
        clear(stackList, data, wfrm, equivKey, DataKeys);

        return WSEML("completed");
    }

    /// [`!', R, O, N]bc
    WSEML logicNot(const WSEML& Args) {
        List* ArgsList = dynamic_cast<List*>(const_cast<Object*>(Args.getRawObject()));
        WSEML* obj = extractObj(ArgsList->find("obj"));
        WSEML* stack = extractObj(ArgsList->find("stack"));
        WSEML* frm = extractObj(ArgsList->find("frm"));
        WSEML* cmd = extractObj(ArgsList->find("cmd"));

        List* cmdList = dynamic_cast<List*>(cmd->getRawObject());
        WSEML& R = cmdList->find("R");
        WSEML& O = cmdList->find("O");

        List* proc = dynamic_cast<List*>(obj->getRawObject());
        List* tables = dynamic_cast<List*>(proc->find("tables").getRawObject());
        WSEML& data_o = proc->find("data");
        List* data = dynamic_cast<List*>(data_o.getRawObject());
        List* uref = dynamic_cast<List*>(tables->find("uref").getRawObject());
        List* readList = dynamic_cast<List*>(uref->find("read").getRawObject());
        List* writeList = dynamic_cast<List*>(uref->find("write").getRawObject());
        List* stackList = dynamic_cast<List*>(stack->getRawObject());
        List* stackInfo = dynamic_cast<List*>(stackList->find("info").getRawObject());
        WSEML* wfrm = &stackInfo->find("wfrm");

        std::string readDll = dynamic_cast<ByteString*>(readList->find("dllName").getRawObject())->get();
        std::string readFunc = dynamic_cast<ByteString*>(readList->find("funcName").getRawObject())->get();
        std::string writeDll = dynamic_cast<ByteString*>(writeList->find("dllName").getRawObject())->get();
        std::string writeFunc = dynamic_cast<ByteString*>(writeList->find("funcName").getRawObject())->get();

        std::string curStackKey = dynamic_cast<ByteString*>(stack->getContainingPair()->getKey().getRawObject())->get();
        std::string curFrmKey = dynamic_cast<ByteString*>(frm->getContainingPair()->getKey().getRawObject())->get();
        data->append(
            &data_o,
            parse(
                "{1:$[t:r]ps, 2:$[t:k, k:stck]ps, 3:$[t:k, k:" + curStackKey + "]ps, 4:$[t:k, k:" + curFrmKey +
                "]ps, 5:$[t:k, k:ip]ps, 6:$[t:i, i:-1]ps}"
            ),
            WSEML("not_tmpPtr")
        );
        data->append(&data_o, NULLOBJ, WSEML("not_O"));
        data->append(&data_o, *cmd, WSEML("not_cmdCopy"));
        data->append(&data_o, NULLOBJ, WSEML("not_res"));

        WSEML equivKey = createEquiv(stack, wfrm, frm, "logicNot", "18");

        std::string procStr = getAddrStr(obj);
        WSEML ipRef = parse("{type:d, 1:$[type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:not_tmpPtr]ps]ptr]ref}");
        WSEML newPs = parse("{type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 2:$[t:k, k:data]ps, 3:$[t:k, k:not_cmdCopy]ps, 4$[t:k, k:N]ps]ptr}");
        ipRef.setSemanticType(WSEML("ref"));
        newPs.setSemanticType(WSEML("ref"));

        std::list<Pair> tmp;
        WSEML refArgs(tmp);
        List* args = dynamic_cast<List*>(const_cast<Object*>(refArgs.getRawObject()));
        args->append(&refArgs, ipRef, WSEML("ref"));
        args->append(&refArgs, newPs, WSEML("data"));
        WSEML res;
        if (cmdList->back() != NULLOBJ) {
            res = callFunc(writeDll.c_str(), writeFunc.c_str(), refArgs);
            if (res == WSEML("stopped"))
                return res;
        }

        changeCommand(stackList, equivKey, "19");

        args->find("ref") = O;
        args->find("data") = parse("{type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:not_O]ps]ptr}");
        res = callFunc(readDll.c_str(), readFunc.c_str(), refArgs);
        if (res == WSEML("stopped"))
            return res;

        WSEML OpArgs = parse(
            "{O:$[type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:not_O]ps]ptr]ref, res:$[type:d, 1:$[comp:$[addr:" +
            procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:not_res]ps]ptr]ref}"
        );
        safeNot(OpArgs);

        changeCommand(stackList, equivKey, "21");

        args->find("ref") = R;
        args->find("data") = parse("{type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:not_res]ps]ptr}");
        res = callFunc(writeDll.c_str(), writeDll.c_str(), refArgs);
        if (res == WSEML("stopped"))
            return res;

        WSEML DataKeys = parse("{1:not_O, 2:not_cmdCopy, 3:not_res, 4:not_tmpPtr}");
        clear(stackList, data, wfrm, equivKey, DataKeys);

        return WSEML("completed");
    }

    /// [‘I’, R, L, RK, K, RD, D, I, N]bc
    /// K is put in the R (if K=$, then generated one)
    WSEML insert(const WSEML& Args) {
        List* ArgsList = dynamic_cast<List*>(const_cast<Object*>(Args.getRawObject()));
        WSEML* obj = extractObj(ArgsList->find("obj"));
        WSEML* stack = extractObj(ArgsList->find("stack"));
        WSEML* frm = extractObj(ArgsList->find("frm"));
        WSEML* cmd = extractObj(ArgsList->find("cmd"));

        List* cmdList = dynamic_cast<List*>(cmd->getRawObject());
        WSEML& R = cmdList->find("R");
        WSEML& L = cmdList->find("L");
        WSEML& RK = cmdList->find("RK");
        WSEML& K = cmdList->find("K");
        WSEML& RD = cmdList->find("RD");
        WSEML& D = cmdList->find("D");
        WSEML& I = cmdList->find("I");

        List* proc = dynamic_cast<List*>(obj->getRawObject());
        List* tables = dynamic_cast<List*>(proc->find("tables").getRawObject());
        WSEML& data_o = proc->find("data");
        List* data = dynamic_cast<List*>(data_o.getRawObject());
        List* uref = dynamic_cast<List*>(tables->find("uref").getRawObject());
        List* readList = dynamic_cast<List*>(uref->find("read").getRawObject());
        List* writeList = dynamic_cast<List*>(uref->find("write").getRawObject());
        List* stackList = dynamic_cast<List*>(stack->getRawObject());
        List* stackInfo = dynamic_cast<List*>(stackList->find("info").getRawObject());
        WSEML* wfrm = &stackInfo->find("wfrm");

        std::string readDll = dynamic_cast<ByteString*>(readList->find("dllName").getRawObject())->get();
        std::string readFunc = dynamic_cast<ByteString*>(readList->find("funcName").getRawObject())->get();
        std::string writeDll = dynamic_cast<ByteString*>(writeList->find("dllName").getRawObject())->get();
        std::string writeFunc = dynamic_cast<ByteString*>(writeList->find("funcName").getRawObject())->get();

        std::string curStackKey = dynamic_cast<ByteString*>(stack->getContainingPair()->getKey().getRawObject())->get();
        std::string curFrmKey = dynamic_cast<ByteString*>(frm->getContainingPair()->getKey().getRawObject())->get();
        data->append(
            &data_o,
            parse(
                "{1:$[t:r]ps, 2:$[t:k, k:stck]ps, 3:$[t:k, k:" + curStackKey + "]ps, 4:$[t:k, k:" + curFrmKey +
                "]ps, 5:$[t:k, k:ip]ps, 6:$[t:i, i:-1]ps}"
            ),
            WSEML("insert_tmpPtr")
        );
        data->append(&data_o, NULLOBJ, WSEML("insert_L"));
        data->append(&data_o, NULLOBJ, WSEML("insert_RK"));
        data->append(&data_o, NULLOBJ, WSEML("insert_K"));
        data->append(&data_o, NULLOBJ, WSEML("insert_RD"));
        data->append(&data_o, NULLOBJ, WSEML("insert_D"));
        data->append(&data_o, NULLOBJ, WSEML("insert_I"));
        data->append(&data_o, *cmd, WSEML("insert_cmdCopy"));
        data->append(&data_o, NULLOBJ, WSEML("insert_res"));

        WSEML equivKey = createEquiv(stack, wfrm, frm, "insert", "23");

        std::string procStr = getAddrStr(obj);
        WSEML ipRef = parse("{type:d, 1:$[type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:insert_tmpPtr]ps]ptr]ref}");
        WSEML newPs = parse("{type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 2:$[t:k, k:data]ps, 3:$[t:k, k:insert_cmdCopy]ps, 4$[t:k, k:N]ps]ptr}");
        ipRef.setSemanticType(WSEML("ref"));
        newPs.setSemanticType(WSEML("ref"));

        std::list<Pair> tmp;
        WSEML refArgs(tmp);
        List* args = dynamic_cast<List*>(const_cast<Object*>(refArgs.getRawObject()));
        args->append(&refArgs, ipRef, WSEML("ref"));
        args->append(&refArgs, newPs, WSEML("data"));
        WSEML res;
        if (cmdList->back() != NULLOBJ) {
            res = callFunc(writeDll.c_str(), writeFunc.c_str(), refArgs);
            if (res == WSEML("stopped"))
                return res;
        }

        changeCommand(stackList, equivKey, "24");
        args->find("ref") = L;
        args->find("data") = parse("{type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:insert_L]ps]ptr}");
        res = callFunc(readDll.c_str(), readFunc.c_str(), refArgs);
        if (res == WSEML("stopped"))
            return res;

        changeCommand(stackList, equivKey, "25");
        args->find("ref") = RK;
        args->find("data") = parse("{type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:insert_RK]ps]ptr}");
        res = callFunc(readDll.c_str(), readFunc.c_str(), refArgs);
        if (res == WSEML("stopped"))
            return res;

        changeCommand(stackList, equivKey, "26");
        args->find("ref") = K;
        args->find("data") = parse("{type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:insert_K]ps]ptr}");
        res = callFunc(readDll.c_str(), readFunc.c_str(), refArgs);
        if (res == WSEML("stopped"))
            return res;

        changeCommand(stackList, equivKey, "27");
        args->find("ref") = RD;
        args->find("data") = parse("{type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:insert_RD]ps]ptr}");
        res = callFunc(readDll.c_str(), readFunc.c_str(), refArgs);
        if (res == WSEML("stopped"))
            return res;

        changeCommand(stackList, equivKey, "28");
        args->find("ref") = D;
        args->find("data") = parse("{type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:insert_D]ps]ptr}");
        res = callFunc(readDll.c_str(), readFunc.c_str(), refArgs);
        if (res == WSEML("stopped"))
            return res;

        changeCommand(stackList, equivKey, "29");
        args->find("ref") = I;
        args->find("data") = parse("{type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:insert_I]ps]ptr}");
        res = callFunc(readDll.c_str(), readFunc.c_str(), refArgs);
        if (res == WSEML("stopped"))
            return res;

        WSEML OpArgs = parse(
            "{L:$[type:d, 1:$[comp:$[addr:" + procStr +
            "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:insert_L]ps]ptr]ref, "
            "RK:$[type:d, 1:$[comp:$[addr:" +
            procStr +
            "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:insert_RK]ps]ptr]ref, "
            "K:$[type:d, 1:$[comp:$[addr:" +
            procStr +
            "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:insert_K]ps]ptr]ref, "
            "RD:$[type:d, 1:$[comp:$[addr:" +
            procStr +
            "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:insert_RD]ps]ptr]ref, "
            "D:$[type:d, 1:$[comp:$[addr:" +
            procStr +
            "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:insert_D]ps]ptr]ref, "
            "I:$[type:d, 1:$[comp:$[addr:" +
            procStr +
            "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:insert_I]ps]ptr]ref, "
            "res:$[type:d, 1:$[comp:$[addr:" +
            procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:insert_res]ps]ptr]ref}"
        );
        safeInsert(OpArgs);

        changeCommand(stackList, equivKey, "31");
        args->find("ref") = L;
        args->find("data") = parse("{type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:insert_L]ps]ptr}");
        res = callFunc(writeDll.c_str(), writeFunc.c_str(), refArgs);
        if (res == WSEML("stopped"))
            return res;

        changeCommand(stackList, equivKey, "32");
        args->find("ref") = R;
        args->find("data") = parse("{type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:insert_res]ps]ptr}");
        res = callFunc(writeDll.c_str(), writeDll.c_str(), refArgs);
        if (res == WSEML("stopped"))
            return res;

        WSEML DataKeys =
            parse("{1:insert_tmpPtr, 2:insert_cmdCopy, 3:insert_L, 4:insert_RK, 5:insert_K, 6:insert_RD, 7:insert_D, 8:insert_I, 9:insert_res}");
        clear(stackList, data, wfrm, equivKey, DataKeys);

        return WSEML("completed");
    }

    /// [‘D’, O, N]bc
    WSEML erase(const WSEML& Args) {
        List* ArgsList = dynamic_cast<List*>(const_cast<Object*>(Args.getRawObject()));
        WSEML* obj = extractObj(ArgsList->find("obj"));
        WSEML* stack = extractObj(ArgsList->find("stack"));
        WSEML* frm = extractObj(ArgsList->find("frm"));
        WSEML* cmd = extractObj(ArgsList->find("cmd"));

        List* cmdList = dynamic_cast<List*>(cmd->getRawObject());
        WSEML& O = cmdList->find("O");

        List* proc = dynamic_cast<List*>(obj->getRawObject());
        List* tables = dynamic_cast<List*>(proc->find("tables").getRawObject());
        WSEML& data_o = proc->find("data");
        List* data = dynamic_cast<List*>(data_o.getRawObject());
        List* uref = dynamic_cast<List*>(tables->find("uref").getRawObject());
        List* readList = dynamic_cast<List*>(uref->find("read").getRawObject());
        List* writeList = dynamic_cast<List*>(uref->find("write").getRawObject());
        List* stackList = dynamic_cast<List*>(stack->getRawObject());
        List* stackInfo = dynamic_cast<List*>(stackList->find("info").getRawObject());
        WSEML* wfrm = &stackInfo->find("wfrm");

        std::string readDll = dynamic_cast<ByteString*>(readList->find("dllName").getRawObject())->get();
        std::string readFunc = dynamic_cast<ByteString*>(readList->find("funcName").getRawObject())->get();
        std::string writeDll = dynamic_cast<ByteString*>(writeList->find("dllName").getRawObject())->get();
        std::string writeFunc = dynamic_cast<ByteString*>(writeList->find("funcName").getRawObject())->get();

        std::string curStackKey = dynamic_cast<ByteString*>(stack->getContainingPair()->getKey().getRawObject())->get();
        std::string curFrmKey = dynamic_cast<ByteString*>(frm->getContainingPair()->getKey().getRawObject())->get();
        data->append(
            &data_o,
            parse(
                "{1:$[t:r]ps, 2:$[t:k, k:stck]ps, 3:$[t:k, k:" + curStackKey + "]ps, 4:$[t:k, k:" + curFrmKey +
                "]ps, 5:$[t:k, k:ip]ps, 6:$[t:i, i:-1]ps}"
            ),
            WSEML("erase_tmpPtr")
        );
        data->append(&data_o, NULLOBJ, WSEML("erase_O"));
        data->append(&data_o, *cmd, WSEML("erase_cmdCopy"));

        WSEML equivKey = createEquiv(stack, wfrm, frm, "erase", "17");

        std::string procStr = getAddrStr(obj);
        WSEML ipRef = parse("{type:d, 1:$[type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:erase_tmpPtr]ps]ptr]ref}");
        WSEML newPs = parse("{type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 2:$[t:k, k:data]ps, 3:$[t:k, k:erase_cmdCopy]ps, 4$[t:k, k:N]ps]ptr}");
        ipRef.setSemanticType(WSEML("ref"));
        newPs.setSemanticType(WSEML("ref"));

        std::list<Pair> tmp;
        WSEML refArgs(tmp);
        List* args = dynamic_cast<List*>(const_cast<Object*>(refArgs.getRawObject()));
        args->append(&refArgs, ipRef, WSEML("ref"));
        args->append(&refArgs, newPs, WSEML("data"));
        WSEML res;
        if (cmdList->back() != NULLOBJ) {
            res = callFunc(writeDll.c_str(), writeFunc.c_str(), refArgs);
            if (res == WSEML("stopped"))
                return res;
        }

        changeCommand(stackList, equivKey, "18");

        args->find("ref") = O;
        args->find("data") = parse("{type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:erase_O]ps]ptr}");
        res = callFunc(readDll.c_str(), readFunc.c_str(), refArgs);
        if (res == WSEML("stopped"))
            return res;

        WSEML OpArgs = parse("{O:$[type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:erase_O]ps]ptr]ref}");
        safeErase(OpArgs);

        WSEML DataKeys = parse("{1:erase_O, 2:erase_cmdCopy, 3:erase_tmpPtr}");
        clear(stackList, data, wfrm, equivKey, DataKeys);

        return WSEML("completed");
    }

    /// [‘E’, R, O, N]bc
    WSEML isDeref(const WSEML& Args) {
        List* ArgsList = dynamic_cast<List*>(const_cast<Object*>(Args.getRawObject()));
        WSEML* obj = extractObj(ArgsList->find("obj"));
        WSEML* stack = extractObj(ArgsList->find("stack"));
        WSEML* frm = extractObj(ArgsList->find("frm"));
        WSEML* cmd = extractObj(ArgsList->find("cmd"));

        List* cmdList = dynamic_cast<List*>(cmd->getRawObject());
        WSEML& O = cmdList->find("O");
        WSEML& R = cmdList->find("R");

        List* proc = dynamic_cast<List*>(obj->getRawObject());
        List* tables = dynamic_cast<List*>(proc->find("tables").getRawObject());
        WSEML& data_o = proc->find("data");
        List* data = dynamic_cast<List*>(data_o.getRawObject());
        List* uref = dynamic_cast<List*>(tables->find("uref").getRawObject());
        List* readList = dynamic_cast<List*>(uref->find("read").getRawObject());
        List* writeList = dynamic_cast<List*>(uref->find("write").getRawObject());
        List* stackList = dynamic_cast<List*>(stack->getRawObject());
        List* stackInfo = dynamic_cast<List*>(stackList->find("info").getRawObject());
        WSEML* wfrm = &stackInfo->find("wfrm");

        std::string readDll = dynamic_cast<ByteString*>(readList->find("dllName").getRawObject())->get();
        std::string readFunc = dynamic_cast<ByteString*>(readList->find("funcName").getRawObject())->get();
        std::string writeDll = dynamic_cast<ByteString*>(writeList->find("dllName").getRawObject())->get();
        std::string writeFunc = dynamic_cast<ByteString*>(writeList->find("funcName").getRawObject())->get();

        std::string curStackKey = dynamic_cast<ByteString*>(stack->getContainingPair()->getKey().getRawObject())->get();
        std::string curFrmKey = dynamic_cast<ByteString*>(frm->getContainingPair()->getKey().getRawObject())->get();
        data->append(
            &data_o,
            parse(
                "{1:$[t:r]ps, 2:$[t:k, k:stck]ps, 3:$[t:k, k:" + curStackKey + "]ps, 4:$[t:k, k:" + curFrmKey +
                "]ps, 5:$[t:k, k:ip]ps, 6:$[t:i, i:-1]ps}"
            ),
            WSEML("isDeref_tmpPtr")
        );
        data->append(&data_o, NULLOBJ, WSEML("isDeref_O"));
        data->append(&data_o, NULLOBJ, WSEML("isDeref_res"));
        data->append(&data_o, *cmd, WSEML("isDeref_cmdCopy"));

        WSEML equivKey = createEquiv(stack, wfrm, frm, "isDeref", "17");

        std::string procStr = getAddrStr(obj);
        WSEML ipRef = parse("{type:d, 1:$[type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:isDeref_tmpPtr]ps]ptr]ref}");
        WSEML newPs = parse("{type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 2:$[t:k, k:data]ps, 3:$[t:k, k:isDeref_cmdCopy]ps, 4$[t:k, k:N]ps]ptr}");
        ipRef.setSemanticType(WSEML("ref"));
        newPs.setSemanticType(WSEML("ref"));
        std::list<Pair> tmp;
        WSEML refArgs(tmp);
        List* args = dynamic_cast<List*>(const_cast<Object*>(refArgs.getRawObject()));
        args->append(&refArgs, ipRef, WSEML("ref"));
        args->append(&refArgs, newPs, WSEML("data"));
        WSEML res;
        if (cmdList->back() != NULLOBJ) {
            res = callFunc(writeDll.c_str(), writeFunc.c_str(), refArgs);
            if (res == WSEML("stopped"))
                return res;
        }

        changeCommand(stackList, equivKey, "18");

        args->find("ref") = O;
        args->find("data") = parse("{type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:isDeref_O]ps]ptr}");
        res = callFunc(readDll.c_str(), readFunc.c_str(), refArgs);
        if (res == WSEML("stopped"))
            return res;

        WSEML OpArgs = parse(
            "{O:$[type:d, 1:$[comp:$[addr:" + procStr +
            "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:isDeref_O]ps]ptr]ref, "
            "res:$[type:d, 1:$[comp:$[addr:" +
            procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:isDeref_res]ps]ptr]ref}"
        );
        safeIsDeref(OpArgs);

        changeCommand(stackList, equivKey, "20");

        args->find("ref") = R;
        args->find("data") = parse("{type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:isDeref_res]ps]ptr}");
        res = callFunc(readDll.c_str(), readFunc.c_str(), refArgs);
        if (res == WSEML("stopped"))
            return res;

        WSEML DataKeys = parse("{1:isDeref_O, 2:isDeref_res, 3:isDeref_cmdCopy, 4:isDeref_tmpPtr}");
        clear(stackList, data, wfrm, equivKey, DataKeys);

        return WSEML("completed");
    }

    /// ['C', R, F, A, N]bc
    WSEML call(const WSEML& Args) {
        List* ArgsList = dynamic_cast<List*>(const_cast<Object*>(Args.getRawObject()));
        WSEML* obj = extractObj(ArgsList->find("obj"));
        WSEML* stack = extractObj(ArgsList->find("stack"));
        WSEML* frm = extractObj(ArgsList->find("frm"));
        WSEML* cmd = extractObj(ArgsList->find("cmd"));

        List* cmdList = dynamic_cast<List*>(cmd->getRawObject());
        WSEML& R = cmdList->find("R");
        WSEML& F = cmdList->find("F");
        WSEML& A = cmdList->find("A");

        List* proc = dynamic_cast<List*>(obj->getRawObject());
        List* tables = dynamic_cast<List*>(proc->find("tables").getRawObject());
        WSEML& data_o = proc->find("data");
        List* data = dynamic_cast<List*>(data_o.getRawObject());
        List* uref = dynamic_cast<List*>(tables->find("uref").getRawObject());
        List* readList = dynamic_cast<List*>(uref->find("read").getRawObject());
        List* writeList = dynamic_cast<List*>(uref->find("write").getRawObject());
        List* stackList = dynamic_cast<List*>(stack->getRawObject());
        List* stackInfo = dynamic_cast<List*>(stackList->find("info").getRawObject());
        WSEML* wfrm = &stackInfo->find("wfrm");

        std::string readDll = dynamic_cast<ByteString*>(readList->find("dllName").getRawObject())->get();
        std::string readFunc = dynamic_cast<ByteString*>(readList->find("funcName").getRawObject())->get();
        std::string writeDll = dynamic_cast<ByteString*>(writeList->find("dllName").getRawObject())->get();
        std::string writeFunc = dynamic_cast<ByteString*>(writeList->find("funcName").getRawObject())->get();

        std::string curStackKey = dynamic_cast<ByteString*>(stack->getContainingPair()->getKey().getRawObject())->get();
        std::string curFrmKey = dynamic_cast<ByteString*>(frm->getContainingPair()->getKey().getRawObject())->get();
        data->append(
            &data_o,
            parse(
                "{1:$[t:r]ps, 2:$[t:k, k:stck]ps, 3:$[t:k, k:" + curStackKey + "]ps, 4:$[t:k, k:" + curFrmKey +
                "]ps, 5:$[t:k, k:ip]ps, 6:$[t:i, i:-1]ps}"
            ),
            WSEML("call_tmpPtr")
        );
        data->append(&data_o, NULLOBJ, WSEML("call_F"));
        data->append(&data_o, NULLOBJ, WSEML("call_A"));
        data->append(&data_o, *cmd, WSEML("call_cmdCopy"));
        data->append(&data_o, NULLOBJ, WSEML("call_res"));

        WSEML equivKey = createEquiv(stack, wfrm, frm, "call", "19");

        std::string procStr = getAddrStr(obj);
        WSEML ipRef = parse("{type:d, 1:$[type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:call_tmpPtr]ps]ptr]ref}");
        WSEML newPs = parse("{type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 2:$[t:k, k:data]ps, 3:$[t:k, k:call_cmdCopy]ps, 4$[t:k, k:N]ps]ptr}");
        ipRef.setSemanticType(WSEML("ref"));
        newPs.setSemanticType(WSEML("ref"));

        std::list<Pair> tmp;
        WSEML refArgs(tmp);
        List* args = dynamic_cast<List*>(const_cast<Object*>(refArgs.getRawObject()));
        args->append(&refArgs, ipRef, WSEML("ref"));
        args->append(&refArgs, newPs, WSEML("data"));
        WSEML res;
        if (cmdList->back() != NULLOBJ) {
            res = callFunc(writeDll.c_str(), writeFunc.c_str(), refArgs);
            if (res == WSEML("stopped"))
                return res;
        }

        changeCommand(stackList, equivKey, "20");

        args->find("ref") = F;
        args->find("data") = parse("{type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:call_F]ps]ptr}");
        res = callFunc(readDll.c_str(), readFunc.c_str(), refArgs);
        if (res == WSEML("stopped"))
            return res;

        changeCommand(stackList, equivKey, "21");

        args->find("ref") = A;
        args->find("data") = parse("{type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:call_A]ps]ptr}");
        res = callFunc(readDll.c_str(), readDll.c_str(), refArgs);
        if (res == WSEML("stopped"))
            return res;

        WSEML OpArgs = parse(
            "{F:$[type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:call_F]ps]ptr]ref, A:$[type:d, 1:$[comp:$[addr:" +
            procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:call_A]ps]ptr]ref, res:$[type:d, 1:$[comp:$[addr:" + procStr +
            "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:call_res]ps]ptr]ref}"
        );
        safeCall(OpArgs);

        changeCommand(stackList, equivKey, "23");

        args->find("ref") = R;
        args->find("data") = parse("{type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:call_res]ps]ptr}");
        res = callFunc(writeDll.c_str(), writeDll.c_str(), refArgs);
        if (res == WSEML("stopped"))
            return res;

        WSEML DataKeys = parse("{1:call_F, 2:call_A, 3:call_cmdCopy, 4:call_res, 5:call_tmpPtr}");
        clear(stackList, data, wfrm, equivKey, DataKeys);

        return WSEML("completed");
    }

    /// [‘P’, O, N]bc
    WSEML lastToI(const WSEML& Args) {
        List* ArgsList = dynamic_cast<List*>(const_cast<Object*>(Args.getRawObject()));
        WSEML* obj = extractObj(ArgsList->find("obj"));
        WSEML* stack = extractObj(ArgsList->find("stack"));
        WSEML* frm = extractObj(ArgsList->find("frm"));
        WSEML* cmd = extractObj(ArgsList->find("cmd"));

        List* cmdList = dynamic_cast<List*>(cmd->getRawObject());
        WSEML& O = cmdList->find("O");

        List* proc = dynamic_cast<List*>(obj->getRawObject());
        List* tables = dynamic_cast<List*>(proc->find("tables").getRawObject());
        WSEML& data_o = proc->find("data");
        List* data = dynamic_cast<List*>(data_o.getRawObject());
        List* uref = dynamic_cast<List*>(tables->find("uref").getRawObject());
        List* readList = dynamic_cast<List*>(uref->find("read").getRawObject());
        List* writeList = dynamic_cast<List*>(uref->find("write").getRawObject());
        List* stackList = dynamic_cast<List*>(stack->getRawObject());
        List* stackInfo = dynamic_cast<List*>(stackList->find("info").getRawObject());
        WSEML* wfrm = &stackInfo->find("wfrm");

        std::string readDll = dynamic_cast<ByteString*>(readList->find("dllName").getRawObject())->get();
        std::string readFunc = dynamic_cast<ByteString*>(readList->find("funcName").getRawObject())->get();
        std::string writeDll = dynamic_cast<ByteString*>(writeList->find("dllName").getRawObject())->get();
        std::string writeFunc = dynamic_cast<ByteString*>(writeList->find("funcName").getRawObject())->get();

        std::string curStackKey = dynamic_cast<ByteString*>(stack->getContainingPair()->getKey().getRawObject())->get();
        std::string curFrmKey = dynamic_cast<ByteString*>(frm->getContainingPair()->getKey().getRawObject())->get();
        data->append(
            &data_o,
            parse(
                "{1:$[t:r]ps, 2:$[t:k, k:stck]ps, 3:$[t:k, k:" + curStackKey + "]ps, 4:$[t:k, k:" + curFrmKey +
                "]ps, 5:$[t:k, k:ip]ps, 6:$[t:i, i:-1]ps}"
            ),
            WSEML("lastToI_tmpPtr")
        );
        data->append(&data_o, NULLOBJ, WSEML("lastToI_O"));
        data->append(&data_o, *cmd, WSEML("lastToI_cmdCopy"));

        WSEML equivKey = createEquiv(stack, wfrm, frm, "lastToI", "17");

        std::string procStr = getAddrStr(obj);
        WSEML ipRef = parse("{type:d, 1:$[type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:lastToI_tmpPtr]ps]ptr]ref}");
        WSEML newPs = parse("{type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 2:$[t:k, k:data]ps, 3:$[t:k, k:lastToI_cmdCopy]ps, 4$[t:k, k:N]ps]ptr}");
        ipRef.setSemanticType(WSEML("ref"));
        newPs.setSemanticType(WSEML("ref"));

        std::list<Pair> tmp;
        WSEML refArgs(tmp);
        List* args = dynamic_cast<List*>(const_cast<Object*>(refArgs.getRawObject()));
        args->append(&refArgs, ipRef, WSEML("ref"));
        args->append(&refArgs, newPs, WSEML("data"));
        WSEML res;
        if (cmdList->back() != NULLOBJ) {
            res = callFunc(writeDll.c_str(), writeFunc.c_str(), refArgs);
            if (res == WSEML("stopped"))
                return res;
        }

        changeCommand(stackList, equivKey, "18");

        args->find("ref") = O;
        args->find("data") = parse("{type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:lastToI_O]ps]ptr}");
        res = callFunc(readDll.c_str(), readFunc.c_str(), refArgs);
        if (res == WSEML("stopped"))
            return res;

        WSEML OpArgs = parse("{O:$[type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:lastToI_O]ps]ptr]ref}");
        safeToI(OpArgs);

        WSEML DataKeys = parse("{1:lastToI_O, 2:lastToI_cmdCopy, 3:lastToI_tmpPtr}");
        clear(stackList, data, wfrm, equivKey, DataKeys);

        return WSEML("completed");
    }

    /// [‘K’, O, N]bc
    WSEML lastToK(const WSEML& Args) {
        List* ArgsList = dynamic_cast<List*>(const_cast<Object*>(Args.getRawObject()));
        WSEML* obj = extractObj(ArgsList->find("obj"));
        WSEML* stack = extractObj(ArgsList->find("stack"));
        WSEML* frm = extractObj(ArgsList->find("frm"));
        WSEML* cmd = extractObj(ArgsList->find("cmd"));

        List* cmdList = dynamic_cast<List*>(cmd->getRawObject());
        WSEML& O = cmdList->find("O");

        List* proc = dynamic_cast<List*>(obj->getRawObject());
        List* tables = dynamic_cast<List*>(proc->find("tables").getRawObject());
        WSEML& data_o = proc->find("data");
        List* data = dynamic_cast<List*>(data_o.getRawObject());
        List* uref = dynamic_cast<List*>(tables->find("uref").getRawObject());
        List* readList = dynamic_cast<List*>(uref->find("read").getRawObject());
        List* writeList = dynamic_cast<List*>(uref->find("write").getRawObject());
        List* stackList = dynamic_cast<List*>(stack->getRawObject());
        List* stackInfo = dynamic_cast<List*>(stackList->find("info").getRawObject());
        WSEML* wfrm = &stackInfo->find("wfrm");

        std::string readDll = dynamic_cast<ByteString*>(readList->find("dllName").getRawObject())->get();
        std::string readFunc = dynamic_cast<ByteString*>(readList->find("funcName").getRawObject())->get();
        std::string writeDll = dynamic_cast<ByteString*>(writeList->find("dllName").getRawObject())->get();
        std::string writeFunc = dynamic_cast<ByteString*>(writeList->find("funcName").getRawObject())->get();

        std::string curStackKey = dynamic_cast<ByteString*>(stack->getContainingPair()->getKey().getRawObject())->get();
        std::string curFrmKey = dynamic_cast<ByteString*>(frm->getContainingPair()->getKey().getRawObject())->get();
        data->append(
            &data_o,
            parse(
                "{1:$[t:r]ps, 2:$[t:k, k:stck]ps, 3:$[t:k, k:" + curStackKey + "]ps, 4:$[t:k, k:" + curFrmKey +
                "]ps, 5:$[t:k, k:ip]ps, 6:$[t:i, i:-1]ps}"
            ),
            WSEML("lastToK_tmpPtr")
        );
        data->append(&data_o, NULLOBJ, WSEML("lastToK_O"));
        data->append(&data_o, *cmd, WSEML("lastToK_cmdCopy"));

        WSEML equivKey = createEquiv(stack, wfrm, frm, "lastToI", "17");

        std::string procStr = getAddrStr(obj);
        WSEML ipRef = parse("{type:d, 1:$[type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:lastToK_tmpPtr]ps]ptr]ref}");
        WSEML newPs = parse("{type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 2:$[t:k, k:data]ps, 3:$[t:k, k:lastToK_cmdCopy]ps, 4$[t:k, k:N]ps]ptr}");
        ipRef.setSemanticType(WSEML("ref"));
        newPs.setSemanticType(WSEML("ref"));

        std::list<Pair> tmp;
        WSEML refArgs(tmp);
        List* args = dynamic_cast<List*>(const_cast<Object*>(refArgs.getRawObject()));
        args->append(&refArgs, ipRef, WSEML("ref"));
        args->append(&refArgs, newPs, WSEML("data"));
        WSEML res;
        if (cmdList->back() != NULLOBJ) {
            res = callFunc(writeDll.c_str(), writeFunc.c_str(), refArgs);
            if (res == WSEML("stopped"))
                return res;
        }

        changeCommand(stackList, equivKey, "18");

        args->find("ref") = O;
        args->find("data") = parse("{type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:lastToK_O]ps]ptr}");
        res = callFunc(readDll.c_str(), readFunc.c_str(), refArgs);
        if (res == WSEML("stopped"))
            return res;

        WSEML OpArgs = parse("{O:$[type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:lastToK_O]ps]ptr]ref}");
        safeToK(OpArgs);

        WSEML DataKeys = parse("{1:lastToK_O, 2:lastToK_cmdCopy, 3:lastToK_tmpPtr}");
        clear(stackList, data, wfrm, equivKey, DataKeys);

        return WSEML("completed");
    }

    /// [`U', R, D, N]
    WSEML callPrevDisp(const WSEML& Args) {
        List* ArgsList = dynamic_cast<List*>(const_cast<Object*>(Args.getRawObject()));
        WSEML* obj = extractObj(ArgsList->find("obj"));
        WSEML* stack = extractObj(ArgsList->find("stack"));
        WSEML* frm = extractObj(ArgsList->find("frm"));
        WSEML* cmd = extractObj(ArgsList->find("cmd"));

        List* cmdList = dynamic_cast<List*>(cmd->getRawObject());
        WSEML& R = cmdList->find("R");
        WSEML& D = cmdList->find("D");

        List* proc = dynamic_cast<List*>(obj->getRawObject());
        List* tables = dynamic_cast<List*>(proc->find("tables").getRawObject());
        WSEML& data_o = proc->find("data");
        List* data = dynamic_cast<List*>(data_o.getRawObject());
        List* uref = dynamic_cast<List*>(tables->find("uref").getRawObject());
        List* readList = dynamic_cast<List*>(uref->find("read").getRawObject());
        List* writeList = dynamic_cast<List*>(uref->find("write").getRawObject());
        List* stackList = dynamic_cast<List*>(stack->getRawObject());
        List* stackInfo = dynamic_cast<List*>(stackList->find("info").getRawObject());
        WSEML* wfrm = &stackInfo->find("wfrm");

        std::string readDll = dynamic_cast<ByteString*>(readList->find("dllName").getRawObject())->get();
        std::string readFunc = dynamic_cast<ByteString*>(readList->find("funcName").getRawObject())->get();
        std::string writeDll = dynamic_cast<ByteString*>(writeList->find("dllName").getRawObject())->get();
        std::string writeFunc = dynamic_cast<ByteString*>(writeList->find("funcName").getRawObject())->get();

        std::string curStackKey = dynamic_cast<ByteString*>(stack->getContainingPair()->getKey().getRawObject())->get();
        std::string curFrmKey = dynamic_cast<ByteString*>(frm->getContainingPair()->getKey().getRawObject())->get();
        data->append(
            &data_o,
            parse(
                "{1:$[t:r]ps, 2:$[t:k, k:stck]ps, 3:$[t:k, k:" + curStackKey + "]ps, 4:$[t:k, k:" + curFrmKey +
                "]ps, 5:$[t:k, k:ip]ps, 6:$[t:i, i:-1]ps}"
            ),
            WSEML("callPrevDisp_tmpPtr")
        );
        data->append(&data_o, NULLOBJ, WSEML("callPrevDisp_D"));
        data->append(&data_o, *cmd, WSEML("callPrevDisp_cmdCopy"));
        data->append(&data_o, NULLOBJ, WSEML("callPrevDisp_res"));

        WSEML equivKey = createEquiv(stack, wfrm, frm, "callPrevDisp", "18");

        std::string procStr = getAddrStr(obj);
        WSEML ipRef =
            parse("{type:d, 1:$[type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:callPrevDisp_tmpPtr]ps]ptr]ref}");
        WSEML newPs =
            parse("{type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 2:$[t:k, k:data]ps, 3:$[t:k, k:callPrevDisp_cmdCopy]ps, 4$[t:k, k:N]ps]ptr}");
        ipRef.setSemanticType(WSEML("ref"));
        newPs.setSemanticType(WSEML("ref"));

        std::list<Pair> tmp;
        WSEML refArgs(tmp);
        List* args = dynamic_cast<List*>(const_cast<Object*>(refArgs.getRawObject()));
        args->append(&refArgs, ipRef, WSEML("ref"));
        args->append(&refArgs, newPs, WSEML("data"));
        WSEML res;
        if (cmdList->back() != NULLOBJ) {
            res = callFunc(writeDll.c_str(), writeFunc.c_str(), refArgs);
            if (res == WSEML("stopped"))
                return res;
        }

        changeCommand(stackList, equivKey, "19");

        args->find("ref") = D;
        args->find("data") = parse("{type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:callPrevDisp_D]ps]ptr}");
        res = callFunc(readDll.c_str(), readFunc.c_str(), refArgs);
        if (res == WSEML("stopped"))
            return res;

        WSEML OpArgs = parse(
            "{stck:$[type:d, 1:$[comp:$[addr:" + procStr +
            "]ptr, 1:$[t:k, k:stck]ps]ptr]ref, "
            "stack:$[type:d, 1:$[comp:$[addr:" +
            procStr + "]ptr, 1:$[t:k, k:stck]ps, 2:$[t:k, k:" + curStackKey +
            "]ps]ptr]ref, "
            "D:$[type:d, 1:$[comp:$[addr:" +
            procStr +
            "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:callPrevDisp_D]ps]ptr]ref, "
            "res:$[type:d, 1:$[comp:$[addr:" +
            procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:callPrevDisp_res]ps]ptr]ref}"
        );
        safeCallPrevDisp(OpArgs);

        changeCommand(stackList, equivKey, "23");

        args->find("ref") = R;
        args->find("data") = parse("{type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:callPrevDisp_res]ps]ptr}");
        res = callFunc(writeDll.c_str(), writeDll.c_str(), refArgs);
        if (res == WSEML("stopped"))
            return res;

        WSEML DataKeys = parse("{1:callPrevDisp_D, 2:callPrevDisp_cmdCopy, 3:callPrevDisp_res, 4:callPrevDisp_tmpPtr}");
        clear(stackList, data, wfrm, equivKey, DataKeys);

        return WSEML("completed");
    }

    /// [`V', R, D, N]
    WSEML callPrevProg(const WSEML& Args) {
        List* ArgsList = dynamic_cast<List*>(const_cast<Object*>(Args.getRawObject()));
        WSEML* obj = extractObj(ArgsList->find("obj"));
        WSEML* stack = extractObj(ArgsList->find("stack"));
        WSEML* frm = extractObj(ArgsList->find("frm"));
        WSEML* cmd = extractObj(ArgsList->find("cmd"));

        List* cmdList = dynamic_cast<List*>(cmd->getRawObject());
        WSEML& R = cmdList->find("R");
        WSEML& D = cmdList->find("D");

        List* proc = dynamic_cast<List*>(obj->getRawObject());
        List* tables = dynamic_cast<List*>(proc->find("tables").getRawObject());
        WSEML& data_o = proc->find("data");
        List* data = dynamic_cast<List*>(data_o.getRawObject());
        List* uref = dynamic_cast<List*>(tables->find("uref").getRawObject());
        List* readList = dynamic_cast<List*>(uref->find("read").getRawObject());
        List* writeList = dynamic_cast<List*>(uref->find("write").getRawObject());
        List* stackList = dynamic_cast<List*>(stack->getRawObject());
        List* stackInfo = dynamic_cast<List*>(stackList->find("info").getRawObject());
        WSEML* wfrm = &stackInfo->find("wfrm");

        std::string readDll = dynamic_cast<ByteString*>(readList->find("dllName").getRawObject())->get();
        std::string readFunc = dynamic_cast<ByteString*>(readList->find("funcName").getRawObject())->get();
        std::string writeDll = dynamic_cast<ByteString*>(writeList->find("dllName").getRawObject())->get();
        std::string writeFunc = dynamic_cast<ByteString*>(writeList->find("funcName").getRawObject())->get();

        std::string curStackKey = dynamic_cast<ByteString*>(stack->getContainingPair()->getKey().getRawObject())->get();
        std::string curFrmKey = dynamic_cast<ByteString*>(frm->getContainingPair()->getKey().getRawObject())->get();
        data->append(
            &data_o,
            parse(
                "{1:$[t:r]ps, 2:$[t:k, k:stck]ps, 3:$[t:k, k:" + curStackKey + "]ps, 4:$[t:k, k:" + curFrmKey +
                "]ps, 5:$[t:k, k:ip]ps, 6:$[t:i, i:-1]ps}"
            ),
            WSEML("callPrevProg_tmpPtr")
        );
        data->append(&data_o, NULLOBJ, WSEML("callPrevProg_D"));
        data->append(&data_o, *cmd, WSEML("callPrevProg_cmdCopy"));
        data->append(&data_o, NULLOBJ, WSEML("callPrevProg_res"));

        WSEML equivKey = createEquiv(stack, wfrm, frm, "callPrevProg", "21");

        std::string procStr = getAddrStr(obj);
        WSEML ipRef =
            parse("{type:d, 1:$[type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:callPrevProg_tmpPtr]ps]ptr]ref}");
        WSEML newPs =
            parse("{type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 2:$[t:k, k:data]ps, 3:$[t:k, k:callPrevProg_cmdCopy]ps, 4$[t:k, k:N]ps]ptr}");
        ipRef.setSemanticType(WSEML("ref"));
        newPs.setSemanticType(WSEML("ref"));

        std::list<Pair> tmp;
        WSEML refArgs(tmp);
        List* args = dynamic_cast<List*>(const_cast<Object*>(refArgs.getRawObject()));
        args->append(&refArgs, ipRef, WSEML("ref"));
        args->append(&refArgs, newPs, WSEML("data"));
        WSEML res;
        if (cmdList->back() != NULLOBJ) {
            res = callFunc(writeDll.c_str(), writeFunc.c_str(), refArgs);
            if (res == WSEML("stopped"))
                return res;
        }

        changeCommand(stackList, equivKey, "22");

        args->find("ref") = D;
        args->find("data") = parse("{type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:callPrevProg_D]ps]ptr}");
        res = callFunc(readDll.c_str(), readFunc.c_str(), refArgs);
        if (res == WSEML("stopped"))
            return res;

        WSEML OpArgs = parse(
            "{stack:$[type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 1:$[t:k, k:stck]ps, 2:$[t:k, k:" + curStackKey +
            "]ps]ptr]ref, "
            "frm:$[type:d, 1:$[comp:$[addr:" +
            procStr + "]ptr, 1:$[t:k, k:stck]ps, 2:$[t:k, k:" + curStackKey + "]ps, 3:$[t:k, k:" + curFrmKey +
            "]ps]ptr]ref, "
            "D:$[type:d, 1:$[comp:$[addr:" +
            procStr +
            "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:callPrevProg_D]ps]ptr]ref, "
            "res:$[type:d, 1:$[comp:$[addr:" +
            procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:callPrevProg_res]ps]ptr]ref}"
        );
        safeCallPrevProg(OpArgs);

        changeCommand(stackList, equivKey, "27");

        args->find("ref") = R;
        args->find("data") = parse("{type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:callPrevProg_res]ps]ptr}");
        res = callFunc(writeDll.c_str(), writeDll.c_str(), refArgs);
        if (res == WSEML("stopped"))
            return res;

        WSEML DataKeys = parse("{1:callPrevProg_D, 2:callPrevProg_cmdCopy, 3:callPrevProg_res, 4:callPrevProg_tmpPtr}");
        clear(stackList, data, wfrm, equivKey, DataKeys);

        return WSEML("completed");
    }

    /// [`T', R, O, N]bc
    WSEML readType(const WSEML& Args) {
        List* ArgsList = dynamic_cast<List*>(const_cast<Object*>(Args.getRawObject()));
        WSEML* obj = extractObj(ArgsList->find("obj"));
        WSEML* stack = extractObj(ArgsList->find("stack"));
        WSEML* frm = extractObj(ArgsList->find("frm"));
        WSEML* cmd = extractObj(ArgsList->find("cmd"));

        List* cmdList = dynamic_cast<List*>(cmd->getRawObject());
        WSEML& O = cmdList->find("O");
        WSEML& R = cmdList->find("R");

        List* proc = dynamic_cast<List*>(obj->getRawObject());
        List* tables = dynamic_cast<List*>(proc->find("tables").getRawObject());
        WSEML& data_o = proc->find("data");
        List* data = dynamic_cast<List*>(data_o.getRawObject());
        List* uref = dynamic_cast<List*>(tables->find("uref").getRawObject());
        List* readList = dynamic_cast<List*>(uref->find("read").getRawObject());
        List* writeList = dynamic_cast<List*>(uref->find("write").getRawObject());
        List* stackList = dynamic_cast<List*>(stack->getRawObject());
        List* stackInfo = dynamic_cast<List*>(stackList->find("info").getRawObject());
        WSEML* wfrm = &stackInfo->find("wfrm");

        std::string readDll = dynamic_cast<ByteString*>(readList->find("dllName").getRawObject())->get();
        std::string readFunc = dynamic_cast<ByteString*>(readList->find("funcName").getRawObject())->get();
        std::string writeDll = dynamic_cast<ByteString*>(writeList->find("dllName").getRawObject())->get();
        std::string writeFunc = dynamic_cast<ByteString*>(writeList->find("funcName").getRawObject())->get();

        std::string curStackKey = dynamic_cast<ByteString*>(stack->getContainingPair()->getKey().getRawObject())->get();
        std::string curFrmKey = dynamic_cast<ByteString*>(frm->getContainingPair()->getKey().getRawObject())->get();
        data->append(
            &data_o,
            parse(
                "{1:$[t:r]ps, 2:$[t:k, k:stck]ps, 3:$[t:k, k:" + curStackKey + "]ps, 4:$[t:k, k:" + curFrmKey +
                "]ps, 5:$[t:k, k:ip]ps, 6:$[t:i, i:-1]ps}"
            ),
            WSEML("readType_tmpPtr")
        );
        data->append(&data_o, NULLOBJ, WSEML("readType_O"));
        data->append(&data_o, NULLOBJ, WSEML("readType_res"));
        data->append(&data_o, *cmd, WSEML("readType_cmdCopy"));

        WSEML equivKey = createEquiv(stack, wfrm, frm, "readType", "18");

        std::string procStr = getAddrStr(obj);
        WSEML ipRef = parse("{type:d, 1:$[type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:readType_tmpPtr]ps]ptr]ref}");
        WSEML newPs = parse("{type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 2:$[t:k, k:data]ps, 3:$[t:k, k:readType_cmdCopy]ps, 4$[t:k, k:N]ps]ptr}");
        ipRef.setSemanticType(WSEML("ref"));
        newPs.setSemanticType(WSEML("ref"));
        std::list<Pair> tmp;
        WSEML refArgs(tmp);
        List* args = dynamic_cast<List*>(const_cast<Object*>(refArgs.getRawObject()));
        args->append(&refArgs, ipRef, WSEML("ref"));
        args->append(&refArgs, newPs, WSEML("data"));
        WSEML res;
        if (cmdList->back() != NULLOBJ) {
            res = callFunc(writeDll.c_str(), writeFunc.c_str(), refArgs);
            if (res == WSEML("stopped"))
                return res;
        }

        changeCommand(stackList, equivKey, "19");

        args->find("ref") = O;
        args->find("data") = parse("{type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:readType_O]ps]ptr}");
        res = callFunc(readDll.c_str(), readFunc.c_str(), refArgs);
        if (res == WSEML("stopped"))
            return res;

        WSEML OpArgs = parse(
            "{O:$[type:d, 1:$[comp:$[addr:" + procStr +
            "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:readType_O]ps]ptr]ref, "
            "res:$[type:d, 1:$[comp:$[addr:" +
            procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:readType_res]ps]ptr]ref}"
        );
        safeReadType(OpArgs);

        changeCommand(stackList, equivKey, "21");

        args->find("ref") = R;
        args->find("data") = parse("{type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:readType_res]ps]ptr}");
        res = callFunc(readDll.c_str(), readFunc.c_str(), refArgs);
        if (res == WSEML("stopped"))
            return res;

        WSEML DataKeys = parse("{1:readType_O, 2:readType_res, 3:readType_cmdCopy, 4:readType_tmpPtr}");
        clear(stackList, data, wfrm, equivKey, DataKeys);

        return WSEML("completed");
    }

    /// [`S', O, T, N]bc
    WSEML setType(const WSEML& Args) {
        List* ArgsList = dynamic_cast<List*>(const_cast<Object*>(Args.getRawObject()));
        WSEML* obj = extractObj(ArgsList->find("obj"));
        WSEML* stack = extractObj(ArgsList->find("stack"));
        WSEML* frm = extractObj(ArgsList->find("frm"));
        WSEML* cmd = extractObj(ArgsList->find("cmd"));

        List* cmdList = dynamic_cast<List*>(cmd->getRawObject());
        WSEML& O = cmdList->find("O");
        WSEML& T = cmdList->find("T");

        List* proc = dynamic_cast<List*>(obj->getRawObject());
        List* tables = dynamic_cast<List*>(proc->find("tables").getRawObject());
        WSEML& data_o = proc->find("data");
        List* data = dynamic_cast<List*>(data_o.getRawObject());
        List* uref = dynamic_cast<List*>(tables->find("uref").getRawObject());
        List* readList = dynamic_cast<List*>(uref->find("read").getRawObject());
        List* writeList = dynamic_cast<List*>(uref->find("write").getRawObject());
        List* stackList = dynamic_cast<List*>(stack->getRawObject());
        List* stackInfo = dynamic_cast<List*>(stackList->find("info").getRawObject());
        WSEML* wfrm = &stackInfo->find("wfrm");

        std::string readDll = dynamic_cast<ByteString*>(readList->find("dllName").getRawObject())->get();
        std::string readFunc = dynamic_cast<ByteString*>(readList->find("funcName").getRawObject())->get();
        std::string writeDll = dynamic_cast<ByteString*>(writeList->find("dllName").getRawObject())->get();
        std::string writeFunc = dynamic_cast<ByteString*>(writeList->find("funcName").getRawObject())->get();

        std::string curStackKey = dynamic_cast<ByteString*>(stack->getContainingPair()->getKey().getRawObject())->get();
        std::string curFrmKey = dynamic_cast<ByteString*>(frm->getContainingPair()->getKey().getRawObject())->get();
        data->append(
            &data_o,
            parse(
                "{1:$[t:r]ps, 2:$[t:k, k:stck]ps, 3:$[t:k, k:" + curStackKey + "]ps, 4:$[t:k, k:" + curFrmKey +
                "]ps, 5:$[t:k, k:ip]ps, 6:$[t:i, i:-1]ps}"
            ),
            WSEML("setType_tmpPtr")
        );
        data->append(&data_o, NULLOBJ, WSEML("setType_O"));
        data->append(&data_o, NULLOBJ, WSEML("setType_T"));
        data->append(&data_o, *cmd, WSEML("setType_cmdCopy"));

        WSEML equivKey = createEquiv(stack, wfrm, frm, "setType", "18");

        std::string procStr = getAddrStr(obj);
        WSEML ipRef = parse("{type:d, 1:$[type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:setType_tmpPtr]ps]ptr]ref}");
        WSEML newPs = parse("{type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 2:$[t:k, k:data]ps, 3:$[t:k, k:setType_cmdCopy]ps, 4$[t:k, k:N]ps]ptr}");
        ipRef.setSemanticType(WSEML("ref"));
        newPs.setSemanticType(WSEML("ref"));
        std::list<Pair> tmp;
        WSEML refArgs(tmp);
        List* args = dynamic_cast<List*>(const_cast<Object*>(refArgs.getRawObject()));
        args->append(&refArgs, ipRef, WSEML("ref"));
        args->append(&refArgs, newPs, WSEML("data"));
        WSEML res;
        if (cmdList->back() != NULLOBJ) {
            res = callFunc(writeDll.c_str(), writeFunc.c_str(), refArgs);
            if (res == WSEML("stopped"))
                return res;
        }

        changeCommand(stackList, equivKey, "19");

        args->find("ref") = O;
        args->find("data") = parse("{type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:setType_O]ps]ptr}");
        res = callFunc(readDll.c_str(), readFunc.c_str(), refArgs);
        if (res == WSEML("stopped"))
            return res;

        changeCommand(stackList, equivKey, "20");
        args->find("ref") = T;
        args->find("data") = parse("{type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:setType_T]ps]ptr}");
        res = callFunc(readDll.c_str(), readFunc.c_str(), refArgs);
        if (res == WSEML("stopped"))
            return res;

        WSEML OpArgs = parse(
            "{O:$[type:d, 1:$[comp:$[addr:" + procStr +
            "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:setType_O]ps]ptr]ref, "
            "T:$[type:d, 1:$[comp:$[addr:" +
            procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:setType_T]ps]ptr]ref}"
        );
        safeSetType(OpArgs);

        changeCommand(stackList, equivKey, "22");
        args->find("ref") = O;
        args->find("data") = parse("{type:d, 1:$[comp:$[addr:" + procStr + "]ptr, 1:$[t:k, k:data]ps, 2:$[t:k, k:setType_O]ps]ptr}");
        res = callFunc(writeDll.c_str(), writeFunc.c_str(), refArgs);
        if (res == WSEML("stopped"))
            return res;

        WSEML DataKeys = parse("{1:setType_O, 2:setType_T, 3:setType_cmdCopy, 4:setType_tmpPtr}");
        clear(stackList, data, wfrm, equivKey, DataKeys);

        return WSEML("completed");
    }
} // namespace wseml