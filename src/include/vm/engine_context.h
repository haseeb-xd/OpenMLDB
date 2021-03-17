/*
 * engine_context.h
 * Copyright (C) 4paradigm 2021 chenjing <chenjing@4paradigm.com>
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef FESQL_ENGINE_CONTEXT_H
#define FESQL_ENGINE_CONTEXT_H
#include "boost/compute/detail/lru_cache.hpp"
namespace fesql {
namespace vm {

enum EngineMode { kBatchMode, kRequestMode, kBatchRequestMode };
std::string EngineModeName(EngineMode mode);

enum ComileType {
    kCompileSQL,
};
class CompileInfo {
 public:
    virtual bool get_ir_buffer(const base::RawBuffer& buf) = 0;
    virtual size_t get_ir_size() = 0;
    virtual const EngineMode GetEngineMode() const = 0;
    virtual const Schema& GetSchema() const = 0;
    virtual const ComileType GetCompileType() const = 0;
    virtual const std::string& GetEncodedSchema() const = 0;
    virtual const Schema& GetRequestSchema() const = 0;
    virtual const std::string& GetRequestName() const = 0;
    virtual void DumpPhysicalPlan(std::ostream& output,
                                  const std::string& tab) = 0;
    virtual void DumpClusterJob(std::ostream& output,
                                const std::string& tab) = 0;
};

typedef std::map<
    EngineMode,
    std::map<std::string, boost::compute::detail::lru_cache<
                              std::string, std::shared_ptr<CompileInfo>>>>
    EngineLRUCache;

class CompileInfoCache {
 public:
    virtual std::shared_ptr<fesql::vm::CompileInfo> GetRequestInfo(
        const std::string& db, const std::string& sp_name,
        base::Status& status) = 0;  // NOLINT
    virtual std::shared_ptr<fesql::vm::CompileInfo> GetBatchRequestInfo(
        const std::string& db, const std::string& sp_name,
        base::Status& status) = 0;  // NOLINT
};

class JITOptions {
 public:
    bool is_enable_mcjit() const { return enable_mcjit_; }
    void set_enable_mcjit(bool flag) { enable_mcjit_ = flag; }

    bool is_enable_vtune() const { return enable_vtune_; }
    void set_enable_vtune(bool flag) { enable_vtune_ = flag; }

    bool is_enable_gdb() const { return enable_gdb_; }
    void set_enable_gdb(bool flag) { enable_gdb_ = flag; }

    bool is_enable_perf() const { return enable_perf_; }
    void set_enable_perf(bool flag) { enable_perf_ = flag; }

 private:
    bool enable_mcjit_ = false;
    bool enable_vtune_ = false;
    bool enable_gdb_ = false;
    bool enable_perf_ = false;
};
}  // namespace vm
}  // namespace fesql
#endif  // FESQL_ENGINE_CONTEXT_H
