using Sift.Models;

namespace Sift.Services;

public interface ISettingsStore
{
    AppSettings Load();
    void Save(AppSettings settings);
}
