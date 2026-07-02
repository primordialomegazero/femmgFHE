with open('src/security/guardian.h', 'r') as f:
    content = f.read()

# Add mutex for SystemMetrics thread safety
old_class = '''class SystemMetrics {
    unsigned long long prev_total = 0, prev_idle = 0;
    struct rusage prev_rusage;'''

new_class = '''class SystemMetrics {
    unsigned long long prev_total = 0, prev_idle = 0;
    struct rusage prev_rusage;
    mutable std::mutex metrics_mutex_;'''

content = content.replace(old_class, new_class)

# Add lock to cpuUsage()
old_cpu = '''    double cpuUsage() {
        std::ifstream stat("/proc/stat");
        if(!stat.is_open()) return 0.5;'''

new_cpu = '''    double cpuUsage() {
        std::lock_guard<std::mutex> lock(metrics_mutex_);
        std::ifstream stat("/proc/stat");
        if(!stat.is_open()) return 0.5;'''

content = content.replace(old_cpu, new_cpu)

# Add lock to processCPU()
old_pcpu = '''    double processCPU() {
        struct rusage usage;
        getrusage(RUSAGE_SELF, &usage);'''

new_pcpu = '''    double processCPU() {
        std::lock_guard<std::mutex> lock(metrics_mutex_);
        struct rusage usage;
        getrusage(RUSAGE_SELF, &usage);'''

content = content.replace(old_pcpu, new_pcpu)

print(f"Fixed thread safety in guardian.h")

with open('src/security/guardian.h', 'w') as f:
    f.write(content)
print("✅ guardian.h — Thread-safe SystemMetrics with mutex")
